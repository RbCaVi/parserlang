#include <stddef.h>
#include "pv/pv.h"

/*
 * The stack
 */

struct pl_stack_cells;

typedef struct {
  struct pl_stack_cells* cells; // pointer to the stack data, refcount, and size
  size_t top; // the number of filled cells
  size_t locals; // the stack index locals start at // also the location of retinfo
} pl_stack;

// stack indexes are positive up from locals or negative down from stack top
// 0 is the retinfo (not used)

pv pl_stack_get(pl_stack,int);
#define pl_stack_top(stack) pl_stack_get(stack,-1)
pl_stack pl_stack_pop(pl_stack);
pl_stack pl_stack_set(pl_stack,pv,int);
pl_stack pl_stack_push(pl_stack,pv);

pl_stack pl_stack_push_frame(pl_stack);
pl_stack pl_stack_pop_frame(pl_stack);

pl_stack pl_stack_ref(pl_stack);
void pl_stack_unref(pl_stack);
