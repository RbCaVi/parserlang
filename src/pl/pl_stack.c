#include "pl/pl_stack.h"

#include "pl/util_pl.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

struct pl_retinfo {
  int locals; // -1 means no return
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

#define stack_cell(stack,i) stack.cells->cells[i]

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

  for (size_t i = 0; i < newstack.top; i++) {
    if (stack_cell(stack,i).type == VAL) {
      pv_copy(stack_cell(stack,stack.top - 1).value);
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

pl_stack pl_stack_new() {
  unsigned int size = 16;
  struct pl_stack_cells *data = malloc(sizeof(struct pl_stack_cells) + size * sizeof(struct pl_stack_cell));
  data->refcount.refcount = 1;
  data->refcount.size = size;
  pl_stack out = {data, 0, 0};
  stack_cell(out,0).type = RET;
  stack_cell(out,0).ret.locals = -1;

  return out;
}

pv pl_stack_get(pl_stack stack,int idx) {
  pv out;

  size_t i = get_stack_idx(stack, idx);

  out = stack_cell(stack,i).value;
  return pv_copy(out);
}

pl_stack pl_stack_set(pl_stack stack,pv val,int idx) {
  // set makes a new stack always
  stack = move_stack(stack);

  size_t i = get_stack_idx(stack, idx);

  pv_free(stack_cell(stack,i).value); // delete the previous value
  stack_cell(stack,i).value = val;

  return stack;
}

pl_stack pl_stack_pop(pl_stack stack) {
  assert(stack_cell(stack,stack.top - 1).type == VAL); // don't pop a retinfo
  pv_free(stack_cell(stack,stack.top - 1).value); // delete the stack top
  stack.top--;
  return stack;
}

pl_stack pl_stack_push(pl_stack stack,pv val) {
  size_t idx;
  // push makes a new stack always
  stack = move_stack(stack);

  inc_size(idx,stack.cells,stack.top,sizeof(struct pl_stack_cells_refcnt),stack.cells->refcount.size,(size_t)((float)stack.cells->refcount.size * 1.5f));
  // initialize the new cell
  stack_cell(stack,idx).type = VAL;
  stack_cell(stack,idx).value = val;

  return stack;
}

pl_stack pl_stack_push_frame(pl_stack stack) {
  size_t idx;
  // push makes a new stack always
  stack = move_stack(stack);
  
  inc_size(idx,stack.cells,stack.top,sizeof(struct pl_stack_cells_refcnt),stack.cells->refcount.size,(size_t)((float)stack.cells->refcount.size * 1.5f));
  // initialize the new cell
  stack_cell(stack,idx).type = RET;
  stack_cell(stack,idx).ret.locals = (int)stack.locals;
  stack.locals = idx;

  return stack;
}

pl_stack pl_stack_pop_frame(pl_stack stack){
  size_t idx = stack.top - 1;
  while (stack_cell(stack,idx).type != RET) {
    pv_free(stack_cell(stack,idx).value);
    idx--;
  }
  // the top of the stack should be a retinfo
  assert(stack_cell(stack,idx).type == RET);
  stack.top = idx;

  return stack;
}

pl_stack pl_stack_split_frame(pl_stack stack, int idx) {
  // split makes a new stack always
  stack = move_stack(stack);

  size_t i = get_stack_idx(stack, idx);

  assert(stack_cell(stack,i).type == VAL);
  pv_free(stack_cell(stack,i).value); // delete the previous value
  stack_cell(stack,i).type = RET;
  stack_cell(stack,i).ret.locals = (int)stack.locals;
  
  stack.locals = idx;

  return stack;
}

pl_stack pl_stack_ref(pl_stack stack){
  stack.cells->refcount.refcount++;
  return stack;
}

void pl_stack_unref(pl_stack stack){
  stack.cells->refcount.refcount--;
  if (stack.cells->refcount.refcount == 0) {
    for (size_t i = 0; i < stack.top; i++) {
      switch (stack_cell(stack,i).type) {
      case RET:
        break;
      case VAL:
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
  inc_size2(idx,parts.data,parts.count,sizeof(size_t),parts.data->size,(uint32_t)((float)parts.data->size * 1.5f), parts.data->parts);
  parts.data->parts[idx].type = KEY;
  parts.data->parts[idx].str = "frames";
  inc_size2(idx,parts.data,parts.count,sizeof(size_t),parts.data->size,(uint32_t)((float)parts.data->size * 1.5f), parts.data->parts);
  parts.data->parts[idx].type = IDX;
  parts.data->parts[idx].idx = frame;
  inc_size2(idx,parts.data,parts.count,sizeof(size_t),parts.data->size,(uint32_t)((float)parts.data->size * 1.5f), parts.data->parts);
  for (size_t i = 0; i < stack.top; i++) {
    switch (stack_cell(stack,i).type) {
    case RET:
      frame++;
      vidx = 0;
      parts.data->parts[idx-1].idx = frame;
      parts.count -= 1;
      print_prefix(parts); // frame.0: frame
      printf("frame %u\n",frame);
      parts.count += 1;
      parts.data->parts[idx].type = KEY;
      parts.data->parts[idx].str = "locals";
      print_prefix(parts); // frame.0.locals: frame
      printf("%i\n",stack_cell(stack,i).ret.locals);
      break;
    case VAL:
      parts.data->parts[idx].type = IDX;
      parts.data->parts[idx].idx = vidx;
      vidx++;
      pl_dump_pv_prefixed(pv_copy(stack_cell(stack,i).value), parts); // frame.0.4: []
      break;
    }
  }
  pl_dump_free_prefix(parts);
}