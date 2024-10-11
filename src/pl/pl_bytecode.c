#include "pl_bytecode.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

pl_bytecode_builder *pl_bytecode_new_builder() {
	pl_bytecode_builder *b = malloc(sizeof(pl_bytecode_builder) + 16);
	b->end = 0;
	b->size = 16;
	return b;
}

pl_bytecode_builder *pl_bytecode_extend(pl_bytecode_builder *b, uint32_t size) {
	b->end += size;
	if (b->end > b->size) {
		uint32_t newsize = (b->size * 3) / 2;
		b = realloc(b, sizeof(pl_bytecode_builder) + newsize);
		b->size = newsize;
	}
	return b;
}

#define OPCODE(op, __data) \
pl_bytecode_builder *pl_bytecode_builder_add_ ## op(pl_bytecode_builder *b, pl_ ## op ## _data data) { \
	uint32_t offset = b->end; \
	b = pl_bytecode_extend(b, sizeof(pl_opcode) + sizeof(pl_ ## op ## _data)); \
	char *pos = b->bytecode + offset; \
	((pl_opcode*)pos)[0] = op; \
	((pl_ ## op ## _data*)(((pl_opcode*)pos) + 1))[0] = data; \
	return b; \
}
#include "pl_opcodes_data.h"
#undef OPCODE

// this is duplicated from pl_exec.c
static pl_opcode plp_get_opcode(const char *bytecode) { \
	return ((pl_opcode*)bytecode)[0]; \
}

// this too
#define OPCODE(op, data) \
static pl_ ## op ## _data plp_get_ ## op ## _data(const char *bytecode) { \
	return ((pl_ ## op ## _data*)(((pl_opcode*)bytecode) + 1))[0]; \
}
#include "pl_opcodes_data.h"
#undef OPCODE

#define opcase(op) \
case(op):; \
	pl_ ## op ## _data op ## _data = plp_get_ ## op ## _data(bytecode); \
	printf(#op); \
	bytecode += sizeof(pl_opcode) + sizeof(pl_ ## op ## _data); \
	(void)op ## _data;

void pl_bytecode_dump(pl_bytecode b) {
	const char *bytecode = b.bytecode;
	const char *end = bytecode + b.length;
	while (bytecode < end) {
		switch (plp_get_opcode(bytecode)) {
			opcase(DUP)
				printf("\n");
				break;
			opcase(ADD)
				printf("\n");
				break;
			opcase(PUSHNUM)
				printf(" %f\n", PUSHNUM_data.n);
				break;
			opcase(SWAPN)
				printf(" %i\n", SWAPN_data.n);
				break;
			opcase(PUSHGLOBAL)
				printf(" %i\n", PUSHGLOBAL_data.i);
				break;
			opcase(CALL)
				printf(" %i\n", CALL_data.n);
				break;
			opcase(RET)
				printf("\n");
				break;
		}
	}
}

pl_bytecode pl_bytecode_from_builder(pl_bytecode_builder *b) {
	char *bytecode = malloc(b->end);
	memcpy(bytecode, b->bytecode, b->end);
	uint32_t len = b->end;
	free(b);
	return (pl_bytecode){bytecode, len};
}

void pl_bytecode_free(pl_bytecode b) {
	free((char*)b.bytecode);
}