#include "plc_codegen.h"

#include <assert.h>
#include <stdlib.h>

// this will be like pv_object in that it has 2 tables - one a linked list and one a list of offsets
// insts is a linked list like slots in pv_pbject

typedef struct {
	int next;
	int prev;
	inst i;
} code_entry;

typedef struct {
	uint32_t refcount;
	uint32_t target;
} jump_entry;

struct code {
	uint32_t alloc_size;
	int first_inst;
	int last_inst;
  int next_free;
  int last_free;
  uint32_t length;
  uint32_t jumpcount;
	uint32_t jumps_alloc_size;
	code_entry insts[];
	//jump_entry[] jumps; // not actually declarable
};

static jump_entry *plcp_code_get_jumps(code *c);

static code *plcp_code_alloc(uint32_t insts_size, uint32_t jumps_size) {
	code *out = malloc(sizeof(code) + sizeof(code_entry) * insts_size + sizeof(jump_entry) * jumps_size);
	out->alloc_size = insts_size;
	out->first_inst = -1;
	out->last_inst = -1;
  out->next_free = 0;
  out->last_free = insts_size - 1;
  out->length = 0;
  out->jumpcount = 0;
	out->jumps_alloc_size = jumps_size;
	for (uint32_t i = 0; i < insts_size; i++) {
		out->insts[i].next = i + 1;
		if (out->insts[i].next >= insts_size) {
			out->insts[i].next = -1;
		}
	}
	jump_entry *jumps = plcp_code_get_jumps(out);
	for (uint32_t i = 0; i < jumps_size; i++) {
		jumps[i].refcount = 0;
	}
	return out;
}

static uint32_t plcp_code_insert_inst(code *out, inst i, int previ) {
	int *prevnext, *nextprev;
	if (previ == -1) {
		prevnext = &(out->first_inst);
	} else {
		prevnext = &(out->insts[previ].next);
	}
	int nexti = *prevnext;
	if (nexti == -1) {
		nextprev = &(out->last_inst);
	} else {
		nextprev = &(out->insts[nexti].prev);
	}
	int newi = out->next_free;
	assert(newi != -1);
	out->next_free = out->insts[newi].next;
	*prevnext = *nextprev = newi;
	out->insts[newi].prev = previ;
	out->insts[newi].next = nexti;
	out->insts[newi].i = i;
	return (uint32_t)newi;
}

int plcp_code_opcode_is_jump(pl_opcode op) {
	switch (op) {
#define OPCODE(op, op_lower, data) \
	case op: \
		return 0; \
		break;
#define JOPCODE(op, op_lower, data) \
	case op: \
		return 1; \
		break;
#include "pl_opcodes_data.h"
#undef OPCODE
#undef JOPCODE
	default:
		abort(); // death
	}
}

static code *plcp_code_insert(code *out, code *in, int idx) {
	assert(out->length + in->length <= out->alloc_size);
	assert(out->jumpcount + in->jumpcount <= out->jumps_alloc_size);

	jump_entry *ijumps = plcp_code_get_jumps(in);
	jump_entry *ojumps = plcp_code_get_jumps(out);
	
	int *jump_translation = malloc(sizeof(int) * in->jumpcount);
	for (uint32_t i = 0; i < in->jumpcount; i++) {
		jump_translation[i] = -1;
		if (ijumps[i].refcount > 0) {
			for (uint32_t j = 0; j < out->jumpcount; j++) {
				if (ojumps[j].refcount == 0 || ojumps[j].target == ijumps[i].target) {
					ojumps[j].refcount += ijumps[i].refcount;
					ojumps[j].target = ijumps[i].target;
					jump_translation[i] = j;
					break;
				}
			}
			assert(jump_translation[i] != -1);
		}
	}

	uint32_t *inst_translation = malloc(sizeof(uint32_t) * in->alloc_size);
  if (in->length > 0) {
		out->first_inst = 0;
		for (int i = in->first_inst; i != -1; i = in->insts[i].next) {
			uint32_t j = plcp_code_insert_inst(out, in->insts[i].i, idx);
			if (plcp_code_opcode_is_jump(out->insts[j].i.opcode)) {
				out->insts[j].i.jump_target = jump_translation[out->insts[j].i.jump_target];
			}
			inst_translation[i] = j;
			idx = j;
		}
  }

	for (uint32_t i = 0; i < in->jumpcount; i++) {
		if (jump_translation[i] != -1) {
			ojumps[jump_translation[i]].target = inst_translation[ojumps[jump_translation[i]].target];
		}
	}

	return out;
}

#define roundup(n) (n + 8) & !(7)

code *plcp_code_realloc(code *in, uint32_t alloc_size, uint32_t jumps_alloc_size) {
	return plcp_code_insert(plcp_code_alloc(alloc_size, jumps_alloc_size), in, -1);
}

code *plcp_code_dup(code *in) {
	return plcp_code_realloc(in, roundup(in->length), roundup(in->jumpcount));
}

code *plcp_code_concat(code *in1, code *in2) {
	return plcp_code_insert(plcp_code_realloc(in2, roundup(in1->length + in2->length), roundup(in1->jumpcount + in2->jumpcount)), in1, -1);
}

pl_bytecode plcp_code_to_bytecode(code *c) {
	uint32_t offsets = malloc(c->length * sizeof(uint32_t));
	pl_bytecode_builder *b = pl_bytecode_new_builder();
	for (int i = c->first_inst; i != -1; i = c->insts[i].next) {
		inst in = c->insts[i].i;
		switch (in.opcode) {
#define OPCODE(op, op_lower, data) \
		case op: \
			pl_bytecode_builder_add(b, op, in.op_lower); \
			break;
#include "pl_opcodes_data.h"
#undef OPCODE
		default:
			abort(); // death
		}
	}
	return pl_bytecode_from_builder(b);
}