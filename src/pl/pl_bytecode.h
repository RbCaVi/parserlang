#ifndef PL_BYTECODE_H
#define PL_BYTECODE_H

#include "pl_opcodes.h"

#include <stdint.h>

typedef struct {
	uint32_t end;
	uint32_t size;
	char bytecode[];
} pl_bytecode_builder;

pl_bytecode_builder *pl_bytecode_new_builder();

#define OPCODE(op, data) \
pl_bytecode_builder *pl_bytecode_builder_add_ ## op(pl_bytecode_builder*, pl_ ## op ## _data);
#include "pl_opcodes_data.h"
#undef OPCODE

void pl_bytecode_dump(const char *bytecode);

#endif