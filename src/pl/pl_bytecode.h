#ifndef PL_BYTECODE_H
#define PL_BYTECODE_H

#include "pl_opcodes.h"

#include <stdint.h>

typedef struct pl_bytecode_builder pl_bytecode_builder;

typedef struct {
	const char *bytecode;
	uint32_t length;
	int freeable;
} pl_bytecode;

pl_bytecode_builder *pl_bytecode_new_builder();

pl_bytecode_builder *pl_bytecode_dup_builder(pl_bytecode_builder*);

#define OPCODE(op, op_lower, data) \
pl_bytecode_builder *pl_bytecode_builder_add_ ## op(pl_bytecode_builder*, pl_ ## op ## _data);
#include "pl_opcodes_data.h"
#undef OPCODE

#define pl_bytecode_builder_add(b, op, data) (b) = pl_bytecode_builder_add_ ## op(b, (pl_ ## op ## _data)data)

uint32_t pl_bytecode_builder_len(pl_bytecode_builder*);

pl_bytecode_builder *pl_bytecode_builder_add_builder(pl_bytecode_builder*, pl_bytecode_builder*);

void pl_bytecode_builder_free(pl_bytecode_builder*);

pl_bytecode pl_bytecode_from_builder(pl_bytecode_builder*);

void pl_bytecode_dump(pl_bytecode);

void pl_bytecode_incref(pl_bytecode);
uint32_t pl_bytecode_getref(pl_bytecode);
void pl_bytecode_free(pl_bytecode);

#endif