#include "pl/pl_stack.h"
#include <assert.h>

pv pl_stack_get(pl_stack stack,int idx) {
  assert(idx != 0);
  if (idx > 0) {
  	assert(locals + idx < top);
  	return stack.cells->cells[locals + idx].value->value; // the longest chain of properties i have ever written
  } else {
  	assert(idx < top);
  	return stack.cells->cells[top - idx].value->value; // the longest chain of properties i have ever written
  }
}

pl_stack pl_stack_pop(pl_stack stack) {
	assert(stack.top > stack.locals + 1); // +1 for the retinfo at the bottom of a frame
	stack.top--;
	return stack;
}

pl_stack pl_stack_set(pl_stack stack,pv val,int idx) {
	// set makes a new stack always
	if (stack.cells->refcnt > 1) {
		
	}
}

pl_stack pl_stack_push(pl_stack stack,pv val) {
	// push makes a new stack always
}

pl_stack pl_stack_push_ref(pl_stack stack,int idx) {
	// push makes a new stack always
}
