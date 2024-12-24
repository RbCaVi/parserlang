#ifndef PL_OPCODES_H
#define PL_OPCODES_H

#define OPCODE(op, op_lower, data) OP_ ## op,
typedef enum {
#include "pl_opcodes_data.h"
} pl_opcode;
#undef OPCODE

#define OPCODE(op, op_lower, data) \
typedef struct data pl_ ## op ## _data; \
typedef pl_ ## op ## _data pl_ ## op_lower ## _data;
#include "pl_opcodes_data.h"
#undef OPCODE

#endif