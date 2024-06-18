#include "pl/pl_stack.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include "pl/util_pl.h"

struct pl_retinfo {
  size_t locals;
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
  size_t refcount; // the number of stacks using these cells
  size_t size; // the number of stack cells
};

struct pl_stack_cells {
  struct pl_stack_cells_refcnt refcount;
  struct pl_stack_cell cells[]; // the stack data
};

pv pl_stack_get(pl_stack stack,int idx) {
  assert(idx != 0);
  pv out;
  size_t i;
  if (idx > 0) {
  	assert(stack.locals + (size_t)idx < stack.top);
  	out = stack.cells->cells[stack.locals + (size_t)idx].value; // the longest chain of properties i have ever written
  } else {
  	assert((size_t)(-idx) < stack.top);
  	out = stack.cells->cells[stack.top - (size_t)(-idx)].value; // the longest chain of properties i have ever written
  }
  return pv_ref(out);
}

pl_stack pl_stack_set(pl_stack stack,pv val,int idx) {
	// set makes a new stack always
	if (stack.cells->refcount.refcount > 1) {
		pl_stack_decref(stack);
		stack = duplicate_stack(stack);
	}
  assert(idx != 0);
  size_t i;
  if (idx > 0) {
  	assert(stack.locals + (size_t)idx < stack.top);
  	i = stack.locals + (size_t)idx;
  } else {
  	assert((size_t)(-idx) < stack.top);
  	i = stack.top - (size_t)(-idx);
  }
  pv_unref(stack.cells->cells[i].value); // delete the previous value
  stack.cells->cells[i].value = val;

  return stack;
}

pl_stack pl_stack_pop(pl_stack stack) {
	assert(stack.cells->cells[stack.top - 1].type == PV); // don't pop a retinfo
	pv_unref(stack.cells->cells[stack.top - 1].value); // delete the stack top
	stack.top--;
	return stack;
}

static pl_stack duplicate_stack(pl_stack stack) {
	pl_stack newstack = stack;
	// duplicate the stack cells
	size_t size = sizeof(struct pl_stack_cells_refcnt) + stack.cells->refcount.size * sizeof(typeof(stack.cells->cells[0]));
	newstack.cells = malloc(size);
  if (newstack.cells == NULL) {
    abort();
  }
	// copy the cells
	memcpy(newstack.cells,stack.cells,size);

	return newstack;
}

pl_stack pl_stack_push(pl_stack stack,pv val) {
	size_t idx;
	// push makes a new stack always
	if (stack.cells->refcount.refcount > 1) {
		pl_stack_decref(stack);
		stack = duplicate_stack(stack);
	}
	inc_size(idx,stack.cells,stack.top,sizeof(struct pl_stack_cells_refcnt),stack.cells->refcount.size,(size_t)((float)stack.cells->refcount.size * 1.5f));
  // initialize the new cell
	stack.cells->cells[idx].value = val;

	return stack;
}

pl_stack pl_stack_push_frame(pl_stack){
	//
}

pl_stack pl_stack_pop_frame(pl_stack){
	//
}

pl_stack pl_stack_ref(pl_stack){
	//
}

void pl_stack_decref(pl_stack){
	// cheese it
	cheese it // don't compile this
}
