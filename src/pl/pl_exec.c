#include "pl_exec.h"

#include "pv.h"
#include "pv_singletons.h"
#include "pv_number.h"
#include "pv_array.h"
#include "pv_object.h"
#include "pl_opcodes.h"
#include "pl_func.h"
#include "pl_iter.h"

#include <stdlib.h>
#include <stdio.h>

static pl_opcode plp_get_opcode(const char *bytecode) { \
	return ((pl_opcode*)bytecode)[0]; \
}

#define OPCODE(op, op_lower, data) \
static pl_ ## op ## _data plp_get_ ## op ## _data(const char *bytecode) { \
	return ((pl_ ## op ## _data*)(((pl_opcode*)bytecode) + 1))[0]; \
}
#include "pl_opcodes_data.h"
#undef OPCODE

#define opcase(op) \
case(op):; \
	pl_ ## op ## _data op ## _data = plp_get_ ## op ## _data(bytecode); \
	bytecode += sizeof(pl_opcode) + sizeof(pl_ ## op ## _data); \
	(void)op ## _data;

void pl_state_set_call(pl_state *state, int argc, const char *ret) {
	pv f = pl_stack_get(state->stack, -(argc + 1));
	state->code = pl_func_get_bytecode(f).bytecode;
	state->stack = pl_stack_split_frame(state->stack, -(argc + 1), state->code);
}

pv pl_next(pl_state *state) {
	const char *bytecode = state->code;
	while (1) {
		switch (plp_get_opcode(bytecode)) {
			opcase(DUP) {
				state->stack = pl_stack_push(state->stack, pl_stack_top(state->stack));
				break;
			}
			opcase(DUPN) {
				state->stack = pl_stack_push(state->stack, pl_stack_get(state->stack, DUPN_data.n));
				break;
			}
			opcase(POP) {
				state->stack = pl_stack_pop(state->stack);
				break;
			}
			opcase(SWAPN) {
				// is that a reference to the hit game ultrakill???
				pv v1 = pl_stack_top(state->stack);
				pv v2 = pl_stack_get(state->stack, -SWAPN_data.n);
				state->stack = pl_stack_set(state->stack, v2, -1);
				state->stack = pl_stack_set(state->stack, v1, -SWAPN_data.n);
				break;
			}
			opcase(SWAPNN) {
				// is that a reference to the hit game ultrakill???
				pv v1 = pl_stack_get(state->stack, -SWAPNN_data.n1);
				pv v2 = pl_stack_get(state->stack, -SWAPNN_data.n2);
				state->stack = pl_stack_set(state->stack, v2, -SWAPNN_data.n1);
				state->stack = pl_stack_set(state->stack, v1, -SWAPNN_data.n2);
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
			opcase(MAKEARRAY) {
				pv a = pv_array();
				for (int i = -(int)MAKEARRAY_data.n; i < 0; i++) {
					a = pv_array_append(a, pl_stack_get(state->stack, i));
				}
				state->stack = pl_stack_popn(state->stack, MAKEARRAY_data.n);
				state->stack = pl_stack_push(state->stack, a);
				break;
			}
			opcase(APPENDA) {
				pv a = pl_stack_get(state->stack, -2);
				pv v = pl_stack_get(state->stack, -1);
				state->stack = pl_stack_popn(state->stack, 2); // have to unref the array from the stack so i can modify it without copying
				a = pv_array_append(a, v);
				state->stack = pl_stack_push(state->stack, a);
				break;
			}
			opcase(CONCATA) {
				pv a1 = pl_stack_get(state->stack, -2);
				pv a2 = pl_stack_get(state->stack, -1);
				state->stack = pl_stack_popn(state->stack, 2); // have to unref the bottom array from the stack so i can modify one without copying
				pv a = pv_array_append(a1, a2);
				state->stack = pl_stack_push(state->stack, a);
				break;
			}
			opcase(SETA) {
				pv a = pl_stack_get(state->stack, -3);
				pv i = pl_stack_get(state->stack, -2);
				pv v = pl_stack_get(state->stack, -1);
				state->stack = pl_stack_popn(state->stack, 3); // have to unref the array from the stack so i can modify it without copying
				a = pv_array_set(a, pv_int_value(i), v);
				state->stack = pl_stack_push(state->stack, a);
				break;
			}
			opcase(SETAI) {
				pv a = pl_stack_get(state->stack, -2);
				pv v = pl_stack_get(state->stack, -1);
				state->stack = pl_stack_popn(state->stack, 2); // have to unref the array from the stack so i can modify it without copying
				a = pv_array_set(a, SETAI_data.i, v);
				state->stack = pl_stack_push(state->stack, a);
				break;
			}
			opcase(GETA) {
				pv a = pl_stack_get(state->stack, -2);
				pv i = pl_stack_get(state->stack, -1);
				state->stack = pl_stack_popn(state->stack, 2);
				pv v = pv_array_get(a, pv_int_value(i));
				state->stack = pl_stack_push(state->stack, v);
				break;
			}
			opcase(GETAI) {
				pv a = pl_stack_get(state->stack, -1);
				state->stack = pl_stack_popn(state->stack, 1);
				pv v = pv_array_get(a, GETAI_data.i);
				state->stack = pl_stack_push(state->stack, v);
				break;
			}
			opcase(LENA) {
				pv a = pl_stack_top(state->stack);
				state->stack = pl_stack_set(state->stack, pv_int((int)pv_array_length(a)), 11);
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
			opcase(SETO) {
				pv o = pl_stack_get(state->stack, -3);
				pv k = pl_stack_get(state->stack, -2);
				pv v = pl_stack_get(state->stack, -1);
				state->stack = pl_stack_popn(state->stack, 3); // have to unref the array from the stack so i can modify it without copying
				o = pv_object_set(o, k, v);
				state->stack = pl_stack_push(state->stack, o);
				break;
			}
			opcase(GETO) {
				pv o = pl_stack_get(state->stack, -2);
				pv k = pl_stack_get(state->stack, -1);
				state->stack = pl_stack_popn(state->stack, 2);
				pv v = pv_object_get(o, k);
				state->stack = pl_stack_push(state->stack, v);
				break;
			}
			opcase(DELO) {
				pv o = pl_stack_get(state->stack, -2);
				pv k = pl_stack_get(state->stack, -1);
				state->stack = pl_stack_popn(state->stack, 2);
				o = pv_object_delete(o, k);
				state->stack = pl_stack_push(state->stack, o);
				break;
			}
			opcase(HASO) {
				pv o = pl_stack_get(state->stack, -2);
				pv k = pl_stack_get(state->stack, -1);
				state->stack = pl_stack_popn(state->stack, 2);
				state->stack = pl_stack_push(state->stack, pv_bool(pv_object_has(o, k)));
				break;
			}
			opcase(LENO) {
				pv o = pl_stack_top(state->stack);
				state->stack = pl_stack_set(state->stack, pv_int((int)pv_object_length(o)), -1);
				break;
			}
			opcase(CALL) {
				printf("in\n");
				if (!pl_func_is_native(pl_stack_get(state->stack, -(CALL_data.n + 1)))) {
					pl_state_set_call(state, CALL_data.n, bytecode);
					bytecode = state->code;
				} else {
					pv f = pl_stack_get(state->stack, -(CALL_data.n + 1));
					pv ret = pl_func_call(f, state);
					state->stack = pl_stack_push(pl_stack_popn(state->stack, CALL_data.n + 1), ret);
				}
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
				pv v = pl_iter_value(pv_copy(i));
				if (pv_get_kind(v) == 0) {
					pv_free(i);
					pv_free(v);
					state->stack = pl_stack_pop(state->stack);
					bytecode += ITERATE_data.target;
				} else {
					state->stack = pl_stack_set(state->stack, pl_iter_next(i), -1);
					state->stack = pl_stack_push(state->stack, v);
				}
				break;
			}
			opcase(RET) {
				printf("out\n");
				if (pl_stack_retaddr(state->stack) == NULL) {
					state->code = bytecode;
					return pl_stack_top(state->stack);
				} else {
					pv val = pl_stack_top(state->stack);
					state->stack = pl_stack_push(pl_stack_pop_frame(state->stack), val);
					bytecode = pl_stack_retaddr(state->stack);
				}
			}
			opcase(GRET) {
				return pv_invalid();
			}
			opcase(SLICEA) {
				abort();
			}
			opcase(SLICEAL) {
				abort();
			}
			opcase(SLICEAM) {
				abort();
			}
			opcase(SLICEAR) {
				abort();
			}
			opcase(SLICEAII) {
				abort();
			}
			opcase(APPENDO) {
				abort();
			}
			default:
				abort(); // how (i think you did something wrong - probably a bad jump offset)
		}
	}
}

pl_state *pl_state_new() {
	pl_state *state = malloc(sizeof(pl_state));
	*state = (pl_state){NULL, NULL, NULL, NULL, pl_stack_new()};
	return state;
}

void pl_state_free(pl_state *state) {
	pl_stack_unref(state->stack);
}