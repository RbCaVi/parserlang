#include "pl_bytecode.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

struct pl_bytecode_builder {
	uint32_t end;
	uint32_t size;
	char bytecode[];
};

pl_bytecode_builder *pl_bytecode_new_builder() {
	pl_bytecode_builder *b = malloc(sizeof(pl_bytecode_builder) + 16);
	b->end = 0;
	b->size = 16;
	return b;
}

pl_bytecode_builder *pl_bytecode_dup_builder(pl_bytecode_builder *b) {
	return pl_bytecode_builder_add_builder(pl_bytecode_new_builder(), b);
}

pl_bytecode_builder *pl_bytecode_extend(pl_bytecode_builder *b, uint32_t size) {
	b->end += size;
	if (b->end > b->size) {
		uint32_t newsize = b->size;
		while (b->end > newsize) {
			newsize = (newsize * 3) / 2;
		}
		b = realloc(b, sizeof(pl_bytecode_builder) + newsize);
		b->size = newsize;
	}
	return b;
}

#define OPCODE(op, op_lower, __data) \
pl_bytecode_builder *pl_bytecode_builder_add_ ## op(pl_bytecode_builder *b, pl_ ## op ## _data data) { \
	uint32_t offset = b->end; \
	b = pl_bytecode_extend(b, sizeof(pl_opcode) + sizeof(pl_ ## op ## _data)); \
	char *pos = b->bytecode + offset; \
	((pl_opcode*)pos)[0] = OPCODE_ ## op; \
	((pl_ ## op ## _data*)(((pl_opcode*)pos) + 1))[0] = data; \
	return b; \
}
#include "pl_opcodes_data.h"
#undef OPCODE

uint32_t pl_bytecode_builder_len(pl_bytecode_builder *b) {
	return b->end;
}

pl_bytecode_builder *pl_bytecode_builder_add_builder(pl_bytecode_builder *b, pl_bytecode_builder *b2) {
	uint32_t offset = b->end;
	b = pl_bytecode_extend(b, b2->end);
	char *pos = b->bytecode + offset;
	memcpy(pos, b2->bytecode, b2->end);

	return b;
}

void pl_bytecode_builder_free(pl_bytecode_builder *b) {
	free(b);
}

// this is duplicated from pl_exec.c
static pl_opcode plp_get_opcode(const char *bytecode) { \
	return ((pl_opcode*)bytecode)[0]; \
}

// this too
#define OPCODE(op, op_lower, data) \
static pl_ ## op ## _data plp_get_ ## op ## _data(const char *bytecode) { \
	return ((pl_ ## op ## _data*)(((pl_opcode*)bytecode) + 1))[0]; \
}
#include "pl_opcodes_data.h"
#undef OPCODE


static int plp_bytecode_instructions_between_(const char *b1, const char *b2) {
	// b1 < b2
	int i = 0;
	while (b1 < b2) {
		switch (plp_get_opcode(b1)) {
#define OPCODE(op, op_lower, data) \
case OPCODE_ ## op: \
	b1 += sizeof(pl_opcode) + sizeof(pl_ ## op ## _data); \
	break;
#include "pl_opcodes_data.h"
#undef OPCODE
		}
		i++;
	}
	return i;
}


static int plp_bytecode_instructions_between(const char *b1, const char *b2) {
	if (b1 > b2) {
		return -plp_bytecode_instructions_between_(b2, b1);
	} else {
		return plp_bytecode_instructions_between_(b1, b2);
	}
}

#define opcase(op) \
case(OPCODE_ ## op):; \
	pl_ ## op ## _data op ## _data = plp_get_ ## op ## _data(bytecode); \
	printf(#op); \
	bytecode += sizeof(pl_opcode) + sizeof(pl_ ## op ## _data); \
	(void)op ## _data;

#define nodataopcase(op) \
opcase(op) \
	printf("\n"); \
	break;

void pl_bytecode_dump(pl_bytecode b) {
	const char *bytecode = b.bytecode;
	const char *end = bytecode + b.length;
	while (bytecode < end) {
		switch (plp_get_opcode(bytecode)) {
			nodataopcase(DUP)
			nodataopcase(POP)
			nodataopcase(PUSHNULL)
			nodataopcase(PUSHSTRING)
			nodataopcase(PUSHARRAY)
			nodataopcase(PUSHOBJECT)
			nodataopcase(APPENDO)
			nodataopcase(DELO)
			nodataopcase(HASO)
			nodataopcase(APPEND)
			nodataopcase(CONCAT)
			nodataopcase(SLICE)
			nodataopcase(LEFT)
			nodataopcase(MID)
			nodataopcase(RIGHT)
			nodataopcase(LEN)
			nodataopcase(SET)
			nodataopcase(GET)
#define UOP(upper_name, lower_name, expr) \
			nodataopcase(upper_name)
#define BOP(upper_name, lower_name, expr, isdefault) \
			nodataopcase(upper_name)
#include "pv_number_ops_data.h"
#undef UOP
#undef BOP
			nodataopcase(EQUAL)
			nodataopcase(ITER)
			nodataopcase(ITERK)
			nodataopcase(ITERV)
			nodataopcase(ITERE)
			nodataopcase(EACH)
			nodataopcase(RET)
			nodataopcase(GRET)
			opcase(DUPN)
				printf(" %i\n", DUPN_data.n);
				break;
			opcase(SETN)
				printf(" %i\n", SETN_data.n);
				break;
			opcase(POPTO)
				printf(" %i\n", POPTO_data.n);
				break;
			opcase(SWAPN)
				printf(" %i\n", SWAPN_data.n);
				break;
			opcase(SWAPNN)
				printf(" %i %i\n", SWAPNN_data.n1, SWAPNN_data.n2);
				break;
			opcase(PUSHBOOL)
				printf(" %s\n", PUSHBOOL_data.v ? "true" : "false");
				break;
			opcase(PUSHINT)
				printf(" %i\n", PUSHINT_data.n);
				break;
			opcase(PUSHDOUBLE)
				printf(" %f\n", PUSHDOUBLE_data.n);
				break;
			opcase(PUSHGLOBAL)
				printf(" %i\n", PUSHGLOBAL_data.i);
				break;
			opcase(MAKEARRAY)
				printf(" %i\n", MAKEARRAY_data.n);
				break;
			opcase(MAKEOBJECT)
				printf(" %i\n", MAKEOBJECT_data.n);
				break;
			opcase(SETI)
				printf(" %i\n", SETI_data.i);
				break;
			opcase(GETI)
				printf(" %i\n", GETI_data.i);
				break;
			opcase(LEFTI)
				printf(" %i\n", LEFTI_data.i);
				break;
			opcase(MIDI)
				printf(" %i\n", MIDI_data.i);
				break;
			opcase(RIGHTI)
				printf(" %i\n", RIGHTI_data.i);
				break;
			opcase(SLICEII)
				printf(" %i %i\n", SLICEII_data.i1, SLICEII_data.i2);
				break;
			opcase(CALL)
				printf(" %i\n", CALL_data.n);
				break;
			opcase(CALLG)
				printf(" %i\n", CALLG_data.n);
				break;
			opcase(JUMP)
				printf(" %i\n", plp_bytecode_instructions_between(bytecode, bytecode + JUMP_data.target));
				break;
			opcase(JUMPIF)
				printf(" %i\n", plp_bytecode_instructions_between(bytecode, bytecode + JUMPIF_data.target));
				break;
			opcase(JUMPIFNOT)
				printf(" %i\n", plp_bytecode_instructions_between(bytecode, bytecode + JUMPIFNOT_data.target));
				break;
			opcase(ITERATE)
				printf(" %i\n", plp_bytecode_instructions_between(bytecode, bytecode + ITERATE_data.target));
				break;
		}
	}
}

pl_bytecode pl_bytecode_from_builder(pl_bytecode_builder *b) {
	uint32_t *refcount = malloc(b->end + sizeof(uint32_t));
	*refcount = 1;
	char *bytecode = (char*)(refcount + 1);
	memcpy(bytecode, b->bytecode, b->end);
	uint32_t len = b->end;
	free(b);
	return (pl_bytecode){bytecode, len, 1};
}

void pl_bytecode_free(pl_bytecode b) {
	if (b.freeable) {
		uint32_t *refcount = (uint32_t*)(b.bytecode) - 1;
		if (--*refcount == 0) {
			free(refcount);
		}
	}
}

void pl_bytecode_incref(pl_bytecode b) {
	if (b.freeable) {
		uint32_t *refcount = (uint32_t*)(b.bytecode) - 1;
		(*refcount)++;
	}
}

uint32_t pl_bytecode_getref(pl_bytecode b) {
	if (b.freeable) {
		uint32_t *refcount = (uint32_t*)(b.bytecode) - 1;
		return *refcount;
	}
	return UINT_MAX;
}