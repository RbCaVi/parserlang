#include "pl/pl_stack.h"

#include <assert.h>
#include <stdlib.h>
#include "pl/util_pl.h"

struct pl_stack_cell_data {
  int refcount; // number of stack cells using this value (references)
  pv value;
};

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
    struct pl_stack_cell_data *value;
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
  if (idx > 0) {
  	assert(stack.locals + (size_t)idx < stack.top);
  	return stack.cells->cells[stack.locals + (size_t)idx].value->value; // the longest chain of properties i have ever written
  } else {
  	assert((size_t)(-idx) < stack.top);
  	return stack.cells->cells[stack.top - (size_t)(-idx)].value->value; // the longest chain of properties i have ever written
  }
}

pl_stack pl_stack_pop(pl_stack stack) {
	assert(stack.top > stack.locals + 1); // +1 for the retinfo at the bottom of a frame
	stack.top--;
	return stack;
}

void replace_pv_pointer(pv *to_replace, pv replacement) {
	pv_ref(replacement);
	pv_unref(*to_replace);
	*to_replace = replacement;
}

static pl_stack duplicate_stack(pl_stack stack) {
	pl_stack newstack = stack;
	// duplicate the stack cells
	size_t size = sizeof(struct pl_stack_cells_refcnt) + stack.cells->refcount.size * sizeof(typeof(stack.cells.cells));
	newstack.cells = malloc
}

pl_stack pl_stack_set(pl_stack stack,pv val,int idx) {
	// set makes a new stack always
	if (stack.cells->refcount.refcount > 1) {
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
  replace_pv_pointer(&(stack.cells->cells[i].value->value), val);

  return stack;
}

pl_stack pl_stack_push(pl_stack stack,pv val) {
	size_t idx;
	// push makes a new stack always
	if (stack.cells->refcount.refcount > 1) {
		stack = duplicate_stack(stack);
	}
	inc_size(idx,stack.cells,stack.top,sizeof(struct pl_stack_cells_refcnt),stack.cells->refcount.size,(size_t)((float)stack.cells->refcount.size * 1.5f));
  // initialize the new cell
	stack.cells->cells[idx].value->refcount = 1;
	stack.cells->cells[idx].value->value = pv_ref(val);

	return stack;
}

pl_stack pl_stack_push_ref(pl_stack stack,int sidx) {
	size_t idx;
	// push makes a new stack always
	if (stack.cells->refcount.refcount > 1) {
		stack = duplicate_stack(stack);
	}
	inc_size(idx,stack.cells,stack.top,sizeof(struct pl_stack_cells_refcnt),stack.cells->refcount.size,(size_t)((float)stack.cells->refcount.size * 1.5f));
  // initialize the new cell
	stack.cells->cells[idx].value = stack.cells->cells[sidx].value;
	stack.cells->cells[idx].value->refcount++;

	return stack;
}
