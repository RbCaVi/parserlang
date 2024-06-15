#include "pl/pl_stack.h"
#include <assert.h>

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
	// push makes a new stack always
	if (stack.cells->refcount > 1) {
		stack = duplicate_stack(stack);
	}
	size_t idx = stack.top;
	stack.top++;
	// allocate more memory
	if (stack.top > stack.size) {

	}
	// initialize the new cell
	stack.cells->cells[idx].value->value = val; // the longest chain of properties i have ever written  }
}

pl_stack pl_stack_push_ref(pl_stack stack,int idx) {
	// push makes a new stack always
}
