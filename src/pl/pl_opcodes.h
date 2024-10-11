#ifndef PL_OPCODES_H
#define PL_OPCODES_H

#define OPCODE(op, data) op,
typedef enum {
#include "pl_opcodes_data.h"
} pl_opcode;
#undef OPCODE

#define OPCODE(op, data) typedef struct data pl_ ## op ## _data;
#include "pl_opcodes_data.h"
#undef OPCODE

#endif