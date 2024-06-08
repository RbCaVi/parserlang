/*
 * The stack
 */

struct pl_stack {
  size_t refcount;
  pl_stack_cell* cells;
  size_t size; // the number of stack cells
  size_t top; // the number of filled cells
  size_t locals; // the stack index locals start at
};

enum pl_stack_cell_type {
  VAL,
  RET
};

struct pl_stack_cell {
  size_t refcount;
  pl_stack_cell_type type;
  union {
    pl_stack_cell_data *value;
    pl_retinfo ret;
  };
};

struct pl_stack_cell_data {
  int refcount;
  pv value;
};

struct pl_retinfo {
  size_t locals;
};

// stack indexes are positive up from locals or negative down from stack top
// 0 is the retinfo (not used)

pv pl_stack_get(pl_stack,size_t);
pv pl_stack_pop(pl_stack);
pl_stack pl_stack_set(pl_stack,pv,size_t);
pl_stack pl_stack_push(pl_stack,pv);
pl_stack pl_stack_push_ref(pl_stack,size_t);
