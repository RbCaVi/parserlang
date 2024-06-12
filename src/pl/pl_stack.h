#include <stddef.h>
#include "pv/pv.h"

/*
 * The stack
 */

struct pl_stack_cell_data {
  int refcount;
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
  size_t refcount;
  enum pl_stack_cell_type type;
  union {
    struct pl_stack_cell_data *value;
    struct pl_retinfo ret;
  };
};

struct pl_stack_cells {
  size_t refcount;
  struct pl_stack_cell cells[];
};

typedef struct {
  struct pl_stack_cells* cells;
  size_t size; // the number of stack cells
  size_t top; // the number of filled cells
  size_t locals; // the stack index locals start at // the location of retinfo
} pl_stack;

// stack indexes are positive up from locals or negative down from stack top
// 0 is the retinfo (not used)

pv pl_stack_get(pl_stack,int);
#define pl_stack_top(stack) pl_stack_get(stack,-1)
pl_stack pl_stack_pop(pl_stack);
pl_stack pl_stack_set(pl_stack,pv,int);
pl_stack pl_stack_push(pl_stack,pv);
pl_stack pl_stack_push_ref(pl_stack,int);
pl_stack pl_stack_add_frame(pl_stack);
pl_stack pl_stack_ref(pl_stack);
void pl_stack_decref(pl_stack);