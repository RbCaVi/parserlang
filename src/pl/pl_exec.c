#include "pl_exec.h"

#include "pv.h"
#include "pv_singletons.h"
#include "pv_number.h"
#include "pv_array.h"
#include "pl_opcodes.h"
#include "pl_func.h"
#include "pl_iter.h"

#include <stdlib.h>

static pl_opcode plp_get_opcode(const char *bytecode) { \
	return ((pl_opcode*)bytecode)[0]; \
}

#define OPCODE(op, data) \
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

pv pl_call(pl_state *state, pl_bytecode f) {
	const char *bytecode = f.bytecode;
	while (1) {
		switch (plp_get_opcode(bytecode)) {
			opcase(DUP) {
				state->stack = pl_stack_push(state->stack, pl_stack_top(state->stack));
				break;
			}
			opcase(POP) {
				state->stack = pl_stack_pop(state->stack);
				break;
			}
			opcase(PUSHNUM) {
				state->stack = pl_stack_push(state->stack, pv_number(PUSHNUM_data.n));
				break;
			}
			opcase(PUSHBOOL) {
				state->stack = pl_stack_push(state->stack, pv_bool(PUSHBOOL_data.v));
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
			opcase(PUSHGLOBAL) {
				state->stack = pl_stack_push(state->stack, pv_copy(state->globals[PUSHGLOBAL_data.i]));
				break;
			}
			opcase(CALL) {
				pv f = pl_stack_get(state->stack, -(CALL_data.n + 1));
				state->stack = pl_stack_split_frame(state->stack, -(CALL_data.n + 1));
				pv ret = pl_func_call(f, state);
				state->stack = pl_stack_push(pl_stack_pop_frame(state->stack), ret);
				break;
			}
			opcase(RET) {
				return pl_stack_top(state->stack);
			}
			opcase(ADD) {
				double v1 = pv_number_value(pl_stack_get(state->stack, -1));
				double v2 = pv_number_value(pl_stack_get(state->stack, -2));
				state->stack = pl_stack_pop(state->stack);
				state->stack = pl_stack_set(state->stack, pv_number(v1 + v2), -1); // avoid pop + push (no reason to)
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
			opcase(ARRAY) {
				pv a = pv_array();
				for (int i = -(int)ARRAY_data.n; i < 0; i++) {
					a = pv_array_append(a, pl_stack_get(state->stack, i));
				}
				state->stack = pl_stack_popn(state->stack, ARRAY_data.n);
				state->stack = pl_stack_push(state->stack, a);
				break;
			}
			opcase(JUMP) {
				bytecode += JUMP_data.target;
				break;
			}
			opcase(APPENDA) {
				pv v = pl_stack_get(state->stack, -1);
				pv a = pl_stack_get(state->stack, -2);
				state->stack = pl_stack_popn(state->stack, 2); // have to unref the array from the stack so i can modify it without copying
				a = pv_array_append(a, v);
				state->stack = pl_stack_push(state->stack, a);
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
			default:
				abort(); // how (i think you did something wrong - probably a bad jump offset)
		}
	}
}