#include "pl_exec.h"

#include "pv.h"
#include "pv_singletons.h"
#include "pv_number.h"
#include "pv_string.h"
#include "pv_array.h"
#include "pv_object.h"
#include "pv_equal.h"
#include "pl_opcodes.h"
#include "pl_func.h"
#include "pl_iter.h"
#include "pl_builtins.h"
#include "pl_dump_state.h"

#include <stdlib.h>
#include <assert.h>
#include <stdbool.h>
//#include <stdio.h>

pl_state *pl_state_new() {
	pl_state *state = malloc(sizeof(pl_state));
	*state = (pl_state){NULL, NULL, pl_stack_new(), NULL, pv_invalid()};
	return state;
}

static void pl_state_save_chain(pl_state *state, pv iter) {
	pl_state *newstate = malloc(sizeof(pl_state));
	*newstate = (pl_state){state->code, state->globals, pl_stack_ref(state->stack), state->saved, state->iter};
	state->saved = newstate;
	state->iter = iter;
}

// no refcounting (nobody will do 300 savepoints in a row and then duplicate the iterator right?)
pl_state *pl_state_dup(pl_state *state) {
	pl_state *newstate = malloc(sizeof(pl_state));
	pl_state *saved = state->saved;
	if (saved != NULL) {
		saved = pl_state_dup(state->saved);
	}
	*newstate = (pl_state){state->code, state->globals, pl_stack_ref(state->stack), saved, pv_copy(state->iter)};
	return newstate;
}

static pl_opcode plp_get_opcode(const char *bytecode) { \
	return ((pl_opcode*)bytecode)[0]; \
}

#define OPCODE(op, op_lower, data) \
static pl_ ## op ## _data plp_get_ ## op ## _data(const char *bytecode) { \
	return ((pl_ ## op ## _data*)(((pl_opcode*)bytecode) + 1))[0]; \
}
#include "pl_opcodes_data.h"
#undef OPCODE

void pl_state_set_call(pl_state *state, int argc) {
	pv f = pl_stack_get(state->stack, -(argc + 1));
	if (!pl_func_is_native(pv_copy(f))) {
		pl_bytecode fcode = pl_func_get_bytecode(pv_copy(f));
		state->stack = pl_stack_split_frame(state->stack, -(argc + 1), state->code, fcode);
		state->code = fcode.bytecode;
	} else {
		state->stack = pl_stack_split_frame(state->stack, -(argc + 1), state->code, (pl_bytecode){NULL, 0, 0});
	}
	state->stack = pl_func_push_closed_vars(f, state->stack);
}

bool gret_impl(pl_state *state) {
	while (true) {
		if (state->saved == NULL) {
			return false;
		}
		//pl_dump_pv(pv_copy(state->iter));
		if (pv_get_kind(state->iter) == 0) {
			pl_state *oldstate = state->saved;
			pl_state oldstate2 = *state;
			*state = *(state->saved);
			*oldstate = oldstate2;
			oldstate->saved = NULL;
			pl_state_free(oldstate);
			return true;
		}
		pv v = pl_iter_value(pv_copy(state->iter));
		if (pv_get_kind(v) != 0) {
			state->code = state->saved->code;
			//printf("stack: %p saved stack A: %p\n", state->stack.cells, state->saved->stack.cells);
			//printf("stack ref: %i saved stack ref: %i\n", *(uint32_t*)(state->stack.cells), *(uint32_t*)(state->saved->stack.cells));
			pl_stack_unref(state->stack);
			state->stack = pl_stack_push(pl_stack_ref(state->saved->stack), v);
			//printf("stack: %p saved stack B: %p\n", state->stack.cells, state->saved->stack.cells);
			//printf("stack ref: %i saved stack ref: %i\n", *(uint32_t*)(state->stack.cells), *(uint32_t*)(state->saved->stack.cells));
			state->iter = pl_iter_next(state->iter);
			return true;
		}
		pl_state *oldstate = state->saved;
		pl_state oldstate2 = *state;
		*state = *(state->saved);
		*oldstate = oldstate2;
		oldstate->saved = NULL;
		pl_state_free(oldstate);
	}
}

#define opcase(op) \
case(OPCODE_ ## op):; \
	printf("\n" #op "\n");/**/ \
	validinstruction = 1; \
	pl_ ## op ## _data op ## _data = plp_get_ ## op ## _data(bytecode); \
	bytecode += sizeof(pl_opcode) + sizeof(pl_ ## op ## _data); \
	(void)op ## _data;

pv pl_next(pl_state *state) {
	//printf("state: (code: %p stack data: %p)\n", state->code, state->stack.cells);
	const char *bytecode = state->code;
	while (1) {
		int validinstruction = 0;
		switch (plp_get_opcode(bytecode)) {
			opcase(DUP) {
				state->stack = pl_stack_push(state->stack, pl_stack_top(state->stack));
				break;
			}
			opcase(DUPN) {
				state->stack = pl_stack_push(state->stack, pl_stack_get(state->stack, DUPN_data.n));
				break;
			}
			opcase(SETN) {
				pv v = pl_stack_top(state->stack);
				state->stack = pl_stack_pop(state->stack);
				state->stack = pl_stack_set(state->stack, v, SETN_data.n);
				break;
			}
			opcase(POP) {
				state->stack = pl_stack_pop(state->stack);
				break;
			}
			opcase(POPTO) {
				state->stack = pl_stack_popto(state->stack, POPTO_data.n);
				break;
			}
			opcase(SWAPN) {
				// is that a reference to the hit game ultrakill???
				pv v1 = pl_stack_top(state->stack);
				pv v2 = pl_stack_get(state->stack, SWAPN_data.n);
				state->stack = pl_stack_set(state->stack, v2, -1);
				state->stack = pl_stack_set(state->stack, v1, SWAPN_data.n);
				break;
			}
			opcase(SWAPNN) {
				// is that a reference to the hit game ultrakill???
				pv v1 = pl_stack_get(state->stack, SWAPNN_data.n1);
				pv v2 = pl_stack_get(state->stack, SWAPNN_data.n2);
				state->stack = pl_stack_set(state->stack, v2, SWAPNN_data.n1);
				state->stack = pl_stack_set(state->stack, v1, SWAPNN_data.n2);
				break;
			}
			opcase(PUSHNULL) {
				state->stack = pl_stack_push(state->stack, pv_null());
				break;
			}
			opcase(PUSHBOOL) {
				state->stack = pl_stack_push(state->stack, pv_bool(PUSHBOOL_data.v));
				break;
			}
			opcase(PUSHINT) {
				state->stack = pl_stack_push(state->stack, pv_int(PUSHINT_data.n));
				break;
			}
			opcase(PUSHDOUBLE) {
				state->stack = pl_stack_push(state->stack, pv_double(PUSHDOUBLE_data.n));
				break;
			}
			opcase(PUSHSTRING) {
				state->stack = pl_stack_push(state->stack, pv_string(""));
				break;
			}
			opcase(PUSHARRAY) {
				state->stack = pl_stack_push(state->stack, pv_array());
				break;
			}
			opcase(PUSHOBJECT) {
				state->stack = pl_stack_push(state->stack, pv_object());
				break;
			}
			opcase(PUSHGLOBAL) {
				state->stack = pl_stack_push(state->stack, pv_copy(state->globals[PUSHGLOBAL_data.i]));
				break;
			}
			opcase(PUSHBUILTIN) {
				state->stack = pl_stack_push(state->stack, pl_func_native(pl_builtins[PUSHBUILTIN_data.i].f));
				break;
			}
			opcase(MAKEARRAY) {
				pv a = pv_array();
				for (int i = -(int)MAKEARRAY_data.n; i < 0; i++) {
					a = pv_array_append(a, pl_stack_get(state->stack, i));
				}
				state->stack = pl_stack_popn(state->stack, MAKEARRAY_data.n);
				state->stack = pl_stack_push(state->stack, a);
				break;
			}
			opcase(MAKEOBJECT) {
				pv o = pv_object();
				for (int i = -(int)MAKEOBJECT_data.n * 2; i < 0; i++) {
					o = pv_object_set(o, pl_stack_get(state->stack, i * 2), pl_stack_get(state->stack, i * 2 + 1));
				}
				state->stack = pl_stack_popn(state->stack, MAKEOBJECT_data.n * 2);
				state->stack = pl_stack_push(state->stack, o);
				break;
			}
			opcase(APPENDO) {
				pv o = pl_stack_get(state->stack, -3);
				pv k = pl_stack_get(state->stack, -2);
				pv v = pl_stack_get(state->stack, -1);
				assert(!pv_object_has(pv_copy(o), pv_copy(k)));
				state->stack = pl_stack_popn(state->stack, 3);
				state->stack = pl_stack_push(state->stack, pv_object_set(o, k, v));
				break;
			}
			opcase(DELO) {
				pv o = pl_stack_get(state->stack, -2);
				pv k = pl_stack_get(state->stack, -1);
				state->stack = pl_stack_popn(state->stack, 2);
				state->stack = pl_stack_push(state->stack, pv_object_delete(o, k));
				break;
			}
			opcase(HASO) {
				pv o = pl_stack_get(state->stack, -2);
				pv k = pl_stack_get(state->stack, -1);
				state->stack = pl_stack_popn(state->stack, 2);
				state->stack = pl_stack_push(state->stack, pv_bool(pv_object_has(o, k)));
				break;
			}
			opcase(APPEND) {
				// not implemented for strings yet :(
				pv a = pl_stack_get(state->stack, -2);
				pv v = pl_stack_get(state->stack, -1);
				state->stack = pl_stack_popn(state->stack, 2);
				state->stack = pl_stack_push(state->stack, pv_array_append(a, v));
				break;
			}
			opcase(CONCAT) {
				// not implemented for strings yet :(
				pv a1 = pl_stack_get(state->stack, -2); // steak saus
				pv a2 = pl_stack_get(state->stack, -1);
				state->stack = pl_stack_popn(state->stack, 2);
				state->stack = pl_stack_push(state->stack, pv_array_concat(a1, a2));
				break;
			}
			opcase(LEN) {
				abort(); // not implemented :(
				break;
			}
			opcase(SET) {
				pv o = pl_stack_get(state->stack, -3);
				pv k = pl_stack_get(state->stack, -2);
				pv v = pl_stack_get(state->stack, -1);
				state->stack = pl_stack_popn(state->stack, 3);
				state->stack = pl_stack_push(state->stack, pv_array_set(o, pv_int_value(k), v));
				break;
			}
			opcase(GET) {
				pv o = pl_stack_get(state->stack, -2);
				pv k = pl_stack_get(state->stack, -1);
				state->stack = pl_stack_popn(state->stack, 2);
				state->stack = pl_stack_push(state->stack, pv_array_get(o, pv_int_value(k)));
				break;
			}
			opcase(CALL) {
				pv f = pl_stack_get(state->stack, -(CALL_data.n + 1)); // the extra copy doesn't matter here because there's (probably) a reference to this function in globals anyway
				state->code = bytecode;
				pl_state_set_call(state, CALL_data.n);
				bytecode = state->code;
				if (pl_func_is_native(pv_copy(f))) {
					pv ret = pl_func_call(f, state);
					state->stack = pl_stack_push(pl_stack_pop_frame(state->stack), ret);
				} else {
					// if f is not native (is pl bytecode), then pl_state_set_call() jumps to it
					// i need to deal with the refcount
					pv_free(f);
				}
				break;
			}
			opcase(CALLG) {
				if (!pl_func_is_native(pl_stack_get(state->stack, -(CALLG_data.n + 1)))) {
					pl_state *pl = pl_state_new();
					pl->globals = state->globals;
					for (int i = -(CALLG_data.n + 1); i < 0; i++) {
						pl->stack = pl_stack_push(pl->stack, pl_stack_get(state->stack, i));
					}
					state->stack = pl_stack_popn(state->stack, (uint32_t)(CALLG_data.n + 1));
					//pl_dump_stack(pl->stack);
					pl_state_set_call(pl, CALLG_data.n);
					pv it = pl_iter_gen(pl);
					state->stack = pl_stack_push(state->stack, it);
				} else {
					// i don't have native generators yet
					// i could do something like lua with callk or something
					// or store the function on the stack?
					abort();
				}
				break;
			}
			opcase(BIND) {
				pv f = pl_stack_get(state->stack, -2);
				pv v = pl_stack_get(state->stack, -1);
				state->stack = pl_stack_popn(state->stack, 2);
				state->stack = pl_stack_push(state->stack, pl_func_add_closure_var(f, v));
				break;
			}
#define UOP(upper_name, lower_name, expr) \
			opcase(upper_name) { \
				pv v1 = pl_stack_get(state->stack, -1); \
				pv v = pv_number_ ## lower_name(v1); \
				state->stack = pl_stack_set(state->stack, v, -1); \
				break; \
			}
#define BOP(upper_name, lower_name, expr, isdefault) \
			opcase(upper_name) { \
				pv v1 = pl_stack_get(state->stack, -2); \
				pv v2 = pl_stack_get(state->stack, -1); \
				pv v = pv_number_ ## lower_name(v1, v2); \
				state->stack = pl_stack_pop(state->stack); \
				state->stack = pl_stack_set(state->stack, v, -1); \
				break; \
			}
#include "pv_number_ops_data.h"
#undef UOP
#undef BOP
			opcase(EQUAL) {
				pv v1 = pl_stack_get(state->stack, -2);
				pv v2 = pl_stack_get(state->stack, -1);
				int v = pv_equal(v1, v2);
				state->stack = pl_stack_pop(state->stack);
				state->stack = pl_stack_set(state->stack, pv_bool(v), -1);
				break;
			}
			opcase(JUMP) {
				bytecode += JUMP_data.target;
				break;
			}
			opcase(JUMPIF) {
				int b = pv_bool_value(pl_stack_top(state->stack));
				if (b) {
					bytecode += JUMPIF_data.target;
				}
				state->stack = pl_stack_pop(state->stack);
				break;
			}
			opcase(JUMPIFNOT) {
				int b = pv_bool_value(pl_stack_top(state->stack));
				if (!b) {
					bytecode += JUMPIFNOT_data.target;
				}
				state->stack = pl_stack_pop(state->stack);
				break;
			}
			opcase(ITER) {
				pv v = pl_stack_top(state->stack);
				pv i = pl_iter(v);
				state->stack = pl_stack_set(state->stack, i, -1);
				break;
			}
			opcase(ITERK) {
				pv v = pl_stack_top(state->stack);
				pv i = pl_iter_keys(v);
				state->stack = pl_stack_set(state->stack, i, -1);
				break;
			}
			opcase(ITERV) {
				pv v = pl_stack_top(state->stack);
				pv i = pl_iter_values(v);
				state->stack = pl_stack_set(state->stack, i, -1);
				break;
			}
			opcase(ITERE) {
				pv v = pl_stack_top(state->stack);
				pv i = pl_iter_entries(v);
				state->stack = pl_stack_set(state->stack, i, -1);
				break;
			}
			opcase(ITERATE) {
				pv i = pl_stack_top(state->stack);
				state->stack = pl_stack_pop(state->stack);
				pv v = pl_iter_value(pv_copy(i));
				//printf("aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa\n");
				//pl_dump_pv(pv_copy(v));
				if (pv_get_kind(v) == 0) {
					//printf("refcount of iterator is ================= %i\n", pv_get_refcount(i));
					pv_free(i);
					pv_free(v);
					//printf("before\n");
					//pl_dump_stack(state->stack);
					//printf("after\n");
					bytecode += ITERATE_data.target;
				} else {
					state->stack = pl_stack_push(state->stack, pl_iter_next(i));
					state->stack = pl_stack_push(state->stack, v);
				}
				break;
			}
			opcase(EACH) {
				// add savepoint, then iterate
				pv i = pl_stack_top(state->stack);
				state->stack = pl_stack_pop(state->stack);
				state->code = bytecode;
				pl_state_save_chain(state, i);
				if (!gret_impl(state)) {
					return pv_invalid();
				}
				bytecode = state->code;
				break;
			}
			opcase(RET) {
				pv ret = pl_stack_top(state->stack);
				//printf("code pos in RET = %p\n", bytecode);
				if (pl_stack_retaddr(state->stack) == NULL) {
					state->code = bytecode;
					//printf("RETURNING!!!!!!\n");
					//pl_dump_stack(state->stack);
					state->stack = pl_stack_pop(state->stack);
					//pl_dump_pv(pv_copy(ret));
					return ret;
				} else {
					//printf("                                     RET from call (not iterator)\n");
					bytecode = pl_stack_retaddr(state->stack);
					state->stack = pl_stack_push(pl_stack_pop_frame(state->stack), ret);
				}
				break;
			}
			opcase(RETS) {
				pv ret = pl_stack_top(state->stack);
				state->stack = pl_stack_pop(state->stack);
				if (pl_stack_retaddr(state->stack) == NULL) {
					state->code = bytecode;
					return ret;
				} else {
					state->code = bytecode;
					pl_state_save_chain(state, pv_invalid());
					bytecode = pl_stack_retaddr(state->stack);
					state->stack = pl_stack_push(pl_stack_pop_frame(state->stack), ret);
				}
				break;
			}
			opcase(GRET) {
				//printf("grert\n");
				if (!gret_impl(state)) {
					return pv_invalid();
				}
				bytecode = state->code;
			}
		}
		//printf("code pos = %p\n", bytecode);
		//pl_dump_stack(state->stack);
		state->code = bytecode;
		pl_dump_state(state);
		if (!validinstruction) {
			abort(); // how (i think you did something wrong - probably a bad jump offset)
		}
	}
}

void pl_state_free(pl_state *state) {
	pl_stack_unref(state->stack);
	if (state->saved != NULL) {
		pl_state_free(state->saved);
	}
	pv_free(state->iter);
	free(state);
}

// typedef struct pl_state {
//   const char *code;
//   //struct pl_state *save; // the last save point - may be NULL - linked list
//   //struct pl_state *parent; // the state that called this generator - may be NULL for top level
// 	pv *globals;
// 	pl_stack stack;
//   struct pl_state *saved;
//   pv iter;
// } pl_state;

static void pl_dump_one_state_prefixed(pl_state *state, pl_dump_prefix parts) {
	parts.count -= 1;
	print_prefixed(parts, "state");
	parts.count += 1;
	//pl_dump_prefix_set_key(parts, idx, "code"); // the code pointer isn't actually very useful (it's currently only set near function calls)
	//print_prefixed(parts, "%p", state->code);
	pl_dump_prefix_set_key(parts, parts.count - 1, "globals");
	pl_dump_pv_prefixed(pv_copy(*state->globals), parts);
	pl_dump_prefix_set_key(parts, parts.count - 1, "stack");
	pl_dump_stack_prefixed(state->stack, parts); // for some reason pl_dump_stack_prefixed() doesn't free the stack
	if (pv_get_kind(state->iter) != 0) {
		pl_dump_prefix_set_key(parts, parts.count - 1, "iterator");
		pl_dump_pv_prefixed(pv_copy(state->iter), parts);
	}
}

void pl_dump_state_prefixed(pl_state *state, pl_dump_prefix parts) {
	parts = pl_dump_dup_prefix(parts);
	pl_dump_prefix_extend(parts);
	pl_dump_one_state_prefixed(state, parts);
	pl_dump_prefix_set_key(parts, parts.count - 1, "saves");
	pl_dump_prefix_extend(parts);
	pl_dump_prefix_extend(parts);
	for (uint32_t i = 1; state != NULL; i++, state = state->saved) {
		pl_dump_prefix_set_idx(parts, parts.count - 2, i);
		pl_dump_one_state_prefixed(state, parts);
	}
	pl_dump_free_prefix(parts);
}