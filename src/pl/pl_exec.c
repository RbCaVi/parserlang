#include "pl_exec.h"

#include "pv.h"
#include "pv_number.h"
#include "pv_singletons.h"
#include "pl_opcodes.h"
#include "pl_func.h"

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
			opcase(DUP)
				abort();
				break;
			opcase(PUSHNUM)
				state->stack = pl_stack_push(state->stack, pv_number(PUSHNUM_data.n));
				break;
			opcase(PUSHBOOL)
				state->stack = pl_stack_push(state->stack, pv_bool(PUSHBOOL_data.v));
				break;
			opcase(SWAPN)
				abort();
				break;
			opcase(PUSHGLOBAL)
				state->stack = pl_stack_push(state->stack, pv_copy(state->globals[PUSHGLOBAL_data.i]));
				break;
			opcase(CALL)
				pv f = pl_stack_get(state->stack, -(CALL_data.n + 1));
				state->stack = pl_stack_split_frame(state->stack, -(CALL_data.n + 1));
				pv ret = pl_func_call(f, state);
				state->stack = pl_stack_push(pl_stack_pop_frame(state->stack), ret);
				break;
			opcase(RET)
				return pl_stack_top(state->stack);
			opcase(ADD)
				double v1 = pv_number_value(pl_stack_get(state->stack, -1));
				double v2 = pv_number_value(pl_stack_get(state->stack, -2));
				state->stack = pl_stack_pop(state->stack);
				state->stack = pl_stack_set(state->stack, pv_number(v1 + v2), -1); // avoid pop + push (no reason to)
				break;
			opcase(JUMPIF)
				int b = pv_bool_value(pl_stack_top(state->stack));
				if (b) {
					bytecode += JUMPIF_data.target;
				}
				state->stack = pl_stack_pop(state->stack);
				break;
			default:
				abort(); // how (i think you did something wrong - probably a jump)
		}
	}
}