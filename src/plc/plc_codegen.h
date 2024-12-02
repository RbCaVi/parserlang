#ifndef PLC_CODEGEN_H
#define PLC_CODEGEN_H

#include "pl_opcodes.h"
#include "pl_bytecode.h"

typedef struct code code;

typedef struct {
	pl_opcode opcode;
	union {
#define OPCODE(op, op_lower, data) \
		pl_ ## op ## _data op_lower;
#include "pl_opcodes_data.h"
#undef OPCODE
	};
	int jump_target;
} inst;

static code *plcp_code_alloc(uint32_t insts_size, uint32_t jumps_size);

static uint32_t plcp_code_insert_inst(code *out, inst i, int previ);

static code *plcp_code_insert(code *out, code *in, int idx);

code *plcp_code_realloc(code *in, uint32_t alloc_size, uint32_t jumps_alloc_size);

code *plcp_code_dup(code *in);

code *plcp_code_concat(code *in1, code *in2);

pl_bytecode plcp_code_to_bytecode(code *c);

#endif