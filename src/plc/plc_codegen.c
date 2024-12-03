struct code {
	enum code_type {
		INST,
		IF,
		IFELSE,
	} type;
};

#define OPCODE(op, op_lower, __data) \
code *plc_code_inst_ ## op_lower(pl_ ## op ## _data data) { \
	code *out = malloc(sizeof(code)); \
	out->type = INST; \
	out->inst.opcode = op; \
	out->inst.op_lower ## _data = data; \
	return out; \
}
#define JOPCODE(op, op_lower, data) /* you do NOT get to make unrestricted jumps */
#include "pl_opcodes_data.h"
#undef OPCODE

code *plc_code_if(code *truecode);
code *plc_code_if_else(code *truecode, code *falsecode);

pl_bytecode pl_code_gen(code *c);