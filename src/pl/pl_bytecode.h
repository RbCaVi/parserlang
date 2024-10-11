#ifndef PL_BYTECODE_H
#define PL_BYTECODE_H

#include "pl_opcodes.h"

#include <stdint.h>

typedef struct {
	uint32_t end;
	uint32_t size;
	char bytecode[];
} pl_bytecode_builder;

typedef struct {
	const char *bytecode;
	uint32_t length;
} pl_bytecode;

pl_bytecode_builder *pl_bytecode_new_builder();

#define OPCODE(op, data) \
pl_bytecode_builder *pl_bytecode_builder_add_ ## op(pl_bytecode_builder*, pl_ ## op ## _data);
#include "pl_opcodes_data.h"
#undef OPCODE

#define pl_bytecode_builder_add(b, op, data) (b) = pl_bytecode_builder_add_ ## op(b, (pl_ ## op ## _data)data)

void pl_bytecode_dump(const char *bytecode);

pl_bytecode pl_bytecode_from_builder(pl_bytecode_builder*);

void pl_bytecode_free(pl_bytecode);

#endif