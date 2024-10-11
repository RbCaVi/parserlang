#include "pl_exec.h"

#include "pv.h"
#include "pv_number.h"
#include "pl_opcodes.h"

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

pv pl_call(pl_state *state, pl_func f) {
	const char *bytecode = f.bytecode.bytecode;
	while (1) {
		switch (plp_get_opcode(bytecode)) {
			opcase(DUP)
				break;
			opcase(PUSHNUM)
				state->stack = pl_stack_push(state->stack, pv_number(PUSHNUM_data.n));
				break;
			opcase(SWAPN)
				abort();
				break;
			opcase(PUSHGLOBAL)
				abort();
				break;
			opcase(CALL)
				abort();
				break;
			opcase(RET)
				return pl_stack_top(state->stack);
			opcase(ADD)
				double v1 = pv_number_value(pl_stack_get(state->stack, -1));
				double v2 = pv_number_value(pl_stack_get(state->stack, -2));
				state->stack = pl_stack_pop(state->stack);
				state->stack = pl_stack_set(state->stack, pv_number(v1 + v2), -1); // avoid pop + push (no reason to)
				break;
		}
	}
}