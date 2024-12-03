#ifndef PLC_CODEGEN_H
#define PLC_CODEGEN_H

typedef struct code code;

#define OPCODE(op, op_lower, __data) \
code *plc_code_inst_ ## op_lower(pl_ ## op ## _data data);
#define JOPCODE(op, op_lower, data) /* you do NOT get to make unrestricted jumps */
#include "pl_opcodes_data.h"
#undef OPCODE

code *plc_code_if(code *truecode);
code *plc_code_if_else(code *truecode, code *falsecode);

pl_bytecode pl_code_gen(code *c);

#endif