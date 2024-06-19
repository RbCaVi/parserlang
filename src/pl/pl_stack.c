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

static size_t get_stack_idx(pl_stack stack,int idx) {
  assert(idx != 0);
  size_t i;
  if (idx > 0) {
    assert(stack.locals + (size_t)idx < stack.top);
    i = stack.locals + (size_t)idx;
  } else {
    assert((size_t)(-idx) <= stack.top);
    i = stack.top - (size_t)(-idx);
  }
  assert(stack.cells->cells[i].type == VAL);

  return i;
}

static pl_stack duplicate_stack(pl_stack stack) {
  pl_stack newstack = stack;
  // duplicate the stack cells
  size_t size = sizeof(struct pl_stack_cells_refcnt) + stack.cells->refcount.size * sizeof(typeof(stack.cells->cells[0]));
  newstack.cells = checked_malloc(size);
  // copy the cells
  memcpy(newstack.cells,stack.cells,size);

  for (size_t i = 0; i < newstack.top; i++) {
    if (stack.cells->cells[i].type == VAL) {
      pv_ref(stack.cells->cells[stack.top - 1].value);
    }
  }

  return newstack;
}

static pl_stack move_stack(pl_stack stack) {
  if (stack.cells->refcount.refcount > 1) {
    pl_stack_unref(stack);
    stack = duplicate_stack(stack);
  }
  return stack;
}

pv pl_stack_get(pl_stack stack,int idx) {
  pv out;

  size_t i = get_stack_idx(stack, idx);

  out = stack.cells->cells[i].value;
  return pv_ref(out);
}

pl_stack pl_stack_set(pl_stack stack,pv val,int idx) {
  // set makes a new stack always
  stack = move_stack(stack);

  size_t i = get_stack_idx(stack, idx);

  pv_unref(stack.cells->cells[i].value); // delete the previous value
  stack.cells->cells[i].value = val;

  return stack;
}

pl_stack pl_stack_pop(pl_stack stack) {
  assert(stack.cells->cells[stack.top - 1].type == VAL); // don't pop a retinfo
  pv_unref(stack.cells->cells[stack.top - 1].value); // delete the stack top
  stack.top--;
  return stack;
}

pl_stack pl_stack_push(pl_stack stack,pv val) {
  size_t idx;
  // push makes a new stack always
  stack = move_stack(stack);

  inc_size(idx,stack.cells,stack.top,sizeof(struct pl_stack_cells_refcnt),stack.cells->refcount.size,(size_t)((float)stack.cells->refcount.size * 1.5f));
  // initialize the new cell
  stack.cells->cells[idx].type = VAL;
  stack.cells->cells[idx].value = val;

  return stack;
}

pl_stack pl_stack_push_frame(pl_stack stack) {
  size_t idx;
  // push makes a new stack always
  stack = move_stack(stack);
  
  inc_size(idx,stack.cells,stack.top,sizeof(struct pl_stack_cells_refcnt),stack.cells->refcount.size,(size_t)((float)stack.cells->refcount.size * 1.5f));
  // initialize the new cell
  stack.cells->cells[idx].type = RET;
  stack.cells->cells[idx].ret.locals = stack.locals;

  return stack;
}

pl_stack pl_stack_pop_frame(pl_stack stack){
  size_t idx = stack.top - 1;
  while (stack.cells->cells[idx].type == VAL) {
    pv_unref(stack.cells->cells[idx].value);
    idx--;
  }
  // the top of the stack should be a retinfo
  assert(stack.cells->cells[idx].type == RET);
  stack.top = idx;

  return stack;
}

pl_stack pl_stack_ref(pl_stack stack){
  stack.cells->refcount.refcount++;
  return stack;
}

void pl_stack_unref(pl_stack stack){
  stack.cells->refcount.refcount--;
  if (stack.cells->refcount.refcount == 0) {
    free(stack.cells);
  }
}
