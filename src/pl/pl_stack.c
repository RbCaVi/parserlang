#include "pl/pl_stack.h"

#include "pl/util_pl.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

struct pl_retinfo {
	const char *ret;
	int locals; // -1 means no return
	pl_bytecode code;
};

enum pl_stack_cell_type {
	VAL,
	RET
};

struct pl_stack_cell {
	enum pl_stack_cell_type type;
	union {
		pv value;
		struct pl_retinfo ret;
	};
};

struct pl_stack_cells_refcnt {
	uint32_t refcount; // the number of stacks using these cells
	uint32_t size; // the number of stack cells
};

struct pl_stack_cells {
	struct pl_stack_cells_refcnt refcount;
	struct pl_stack_cell cells[]; // the stack data
};

#define stack_cell(stack,i) stack.cells->cells[i]

static uint32_t get_stack_idx(pl_stack stack,int idx) {
	assert(idx != 0);
	uint32_t i;
	if (idx > 0) {
		assert(stack.locals + (uint32_t)idx < stack.top);
		i = stack.locals + (uint32_t)idx;
	} else {
		assert((uint32_t)(-idx) <= stack.top);
		i = stack.top - (uint32_t)(-idx);
	}
	assert(stack_cell(stack,i).type == VAL);

	return i;
}

static pl_stack duplicate_stack(pl_stack stack) {
	pl_stack newstack = stack;
	// duplicate the stack cells
	size_t size = sizeof(struct pl_stack_cells_refcnt) + stack.cells->refcount.size * sizeof(typeof(stack_cell(stack,0)));
	newstack.cells = checked_malloc(size);
	// copy the cells
	memcpy(newstack.cells,stack.cells,size);

	//printf("stack dup size %i\n", size);

	for (uint32_t i = 0; i < newstack.top; i++) {
		switch (stack_cell(stack,i).type) {
		case RET:
			//printf("stack ret %i\n", i);
			//printf("bytecode dup refcount 1 = %i\n", pl_bytecode_getref(stack_cell(stack, i).ret.code));
			pl_bytecode_incref(stack_cell(stack, i).ret.code);
			//printf("bytecode dup refcount 2 %p = %i\n", stack_cell(stack, i).ret.code.bytecode, pl_bytecode_getref(stack_cell(stack, i).ret.code));
			break;
		case VAL:
			pv_copy(stack_cell(stack, i).value);
			break;
		}
	}
	newstack.cells->refcount.refcount = 1;
	//printf("================================================= %p duped to %i\n", newstack.cells, newstack.cells->refcount.refcount);

	return newstack;
}

static pl_stack move_stack(pl_stack stack) {
	if (stack.cells->refcount.refcount > 1) {
		pl_stack_unref(stack);
		stack = duplicate_stack(stack);
	}
	return stack;
}

pl_stack pl_stack_new() {
	unsigned int size = 16;
	//printf("init size: %li", sizeof(struct pl_stack_cells) + size * sizeof(struct pl_stack_cell));
	struct pl_stack_cells *data = malloc(sizeof(struct pl_stack_cells) + size * sizeof(struct pl_stack_cell));
	data->refcount.refcount = 1;
	data->refcount.size = size;
	pl_stack out = {data, 0, 0};
	stack_cell(out,0).type = RET;
	stack_cell(out,0).ret.locals = -1;
	stack_cell(out,0).ret.code = (pl_bytecode){NULL, 0, 0};
	//printf("================================================= %p new to %i\n", out.cells, out.cells->refcount.refcount);
	
	return out;
}

pv pl_stack_get(pl_stack stack,int idx) {
	pv out;

	uint32_t i = get_stack_idx(stack, idx);

	out = stack_cell(stack,i).value;
	return pv_copy(out);
}

pl_stack pl_stack_set(pl_stack stack,pv val,int idx) {
	// set makes a new stack always
	stack = move_stack(stack);

	uint32_t i = get_stack_idx(stack, idx);

	pv_free(stack_cell(stack,i).value); // delete the previous value
	stack_cell(stack,i).value = val;

	return stack;
}

pl_stack pl_stack_popn(pl_stack stack, uint32_t n) {
	stack = move_stack(stack); // i actually need this
	for (uint32_t i = stack.top - n; i < stack.top; i++) {
		assert(stack_cell(stack, i).type == VAL); // don't pop a retinfo
		pv_free(stack_cell(stack, i).value); // delete the stack top
	}
	stack.top -= n;
	return stack;
}

// pop as many elements as needed to make the current frame n elements
pl_stack pl_stack_popto(pl_stack stack, uint32_t n) {
	stack = move_stack(stack); // i actually need this
	for (uint32_t i = stack.top - 1; i >= stack.locals + n; i--) {
		assert(stack_cell(stack, i).type == VAL); // don't pop a retinfo
		pv_free(stack_cell(stack, i).value); // delete the stack top
	}
	stack.top = stack.locals + n;
	return stack;
}

pl_stack pl_stack_push(pl_stack stack,pv val) {
	uint32_t idx;
	// push makes a new stack always
	stack = move_stack(stack);

	//printf("stack pointer: %p\n", stack.cells);

	inc_size2(idx,stack.cells,stack.top,sizeof(struct pl_stack_cells_refcnt),stack.cells->refcount.size,(uint32_t)((float)stack.cells->refcount.size * 1.5f), stack.cells->cells);
	// initialize the new cell
	stack_cell(stack,idx).type = VAL;
	stack_cell(stack,idx).value = val;

	return stack;
}

//pl_stack pl_stack_push_frame(pl_stack stack) {
//	uint32_t idx;
//	// push makes a new stack always
//	stack = move_stack(stack);
//	
//	inc_size2(idx,stack.cells,stack.top,sizeof(struct pl_stack_cells_refcnt),stack.cells->refcount.size,(uint32_t)((float)stack.cells->refcount.size * 1.5f), stack.cells->cells);
//	// initialize the new cell
//	stack_cell(stack,idx).type = RET;
//	stack_cell(stack,idx).ret.locals = (int)stack.locals;
//	stack.locals = idx;
//
//	return stack;
//}

const char *pl_stack_retaddr(pl_stack stack){
	uint32_t idx = stack.top - 1;
	while (stack_cell(stack,idx).type != RET) {
		idx--;
	}
	// the top of the stack should be a retinfo
	assert(stack_cell(stack,idx).type == RET);

	return stack_cell(stack,idx).ret.ret;
}

pl_stack pl_stack_pop_frame(pl_stack stack){
	// pop_frame makes a new stack always
	stack = move_stack(stack);

	uint32_t idx = stack.top - 1;
	while (stack_cell(stack,idx).type != RET) {
		pv_free(stack_cell(stack,idx).value);
		idx--;
	}
	// the top of the stack should be a retinfo
	assert(stack_cell(stack,idx).type == RET);
	stack.top = idx;
	stack.locals = (uint32_t)stack_cell(stack,idx).ret.locals;
	//printf("bytecode refcount a = %i\n", pl_bytecode_getref(stack_cell(stack,idx).ret.code));
	pl_bytecode_free(stack_cell(stack,idx).ret.code);
	//printf("bytecode refcount b %p = %i\n", stack_cell(stack,idx).ret.code.bytecode, pl_bytecode_getref(stack_cell(stack,idx).ret.code));

	return stack;
}

pl_stack pl_stack_split_frame(pl_stack stack, int idx, const char *ret, pl_bytecode fcode) {
	// split makes a new stack always
	stack = move_stack(stack);

	uint32_t i = get_stack_idx(stack, idx);

	assert(stack_cell(stack,i).type == VAL);
	//printf("bytecode refcount 1 = %i\n", pl_bytecode_getref(fcode));
	pv_free(stack_cell(stack,i).value); // delete the previous value
	stack_cell(stack,i).type = RET;
	stack_cell(stack,i).ret.locals = (int)stack.locals;
	stack_cell(stack,i).ret.ret = ret;
	//printf("bytecode refcount 2 %p = %i at %i\n", fcode.bytecode, pl_bytecode_getref(fcode), i);
	stack_cell(stack,i).ret.code = fcode;

	stack.locals = i;

	return stack;
}

pl_stack pl_stack_ref(pl_stack stack){
	//printf("-------------------------------REF\n");
	stack.cells->refcount.refcount++;
	//printf("================================================= %p refed to %i\n", stack.cells, stack.cells->refcount.refcount);
	return stack;
}

void pl_stack_unref(pl_stack stack){
	stack.cells->refcount.refcount--;
	//printf("================================================= %p unrefed to %i\n", stack.cells, stack.cells->refcount.refcount);
	if (stack.cells->refcount.refcount == 0) {
		//printf("============================================================= real free\n");
		//printf("-------------------------------UNREF\n");
		for (uint32_t i = 0; i < stack.top; i++) {
			switch (stack_cell(stack,i).type) {
			case RET:
				//printf("bytecode refcount free = %i at %i\n", pl_bytecode_getref(stack_cell(stack,i).ret.code));
				//printf("bytecode refcount A %p = %i at %i\n", stack_cell(stack,i).ret.code.bytecode, pl_bytecode_getref(stack_cell(stack,i).ret.code), i);
				pl_bytecode_free(stack_cell(stack,i).ret.code);
				//printf("bytecode refcount B = %i at %i\n", pl_bytecode_getref(stack_cell(stack,i).ret.code), i);
				break;
			case VAL:
				//printf("============================================================= pv refcount free = %i at %i\n", pv_get_refcount(stack_cell(stack,i).value), i);
				//pl_dump_pv(pv_copy(stack_cell(stack,i).value));
				pv_free(stack_cell(stack,i).value);
				break;
			}
		}
		free(stack.cells);
	}
}

void pl_dump_stack_prefixed(pl_stack stack, pl_dump_prefix parts) {
	print_prefix(parts);
	printf("stack\n");
	uint32_t frame = 0;
	uint32_t vidx = 0;
	uint32_t idx;
	parts = pl_dump_dup_prefix(parts);
	pl_dump_prefix_extend(parts);
	pl_dump_prefix_extend(parts);
	for (uint32_t i = 0; i < stack.top; i++) {
		pl_dump_prefix_set_idx(parts, parts.count - 2, frame);
		switch (stack_cell(stack,i).type) {
		case RET:
			frame++;
			vidx = 0;
			parts.count -= 1;
			print_prefixed(parts, "frame %u", frame);
			parts.count += 1;
			pl_dump_prefix_set_key(parts, parts.count - 1, "locals");
			print_prefixed(parts, "%i", stack_cell(stack,i).ret.locals);
			break;
		case VAL:
			pl_dump_prefix_set_idx(parts, parts.count - 1, vidx);
			vidx++;
			pl_dump_pv_prefixed(pv_copy(stack_cell(stack,i).value), parts); // frame.0.4: []
			break;
		}
	}
	pl_dump_free_prefix(parts);
}