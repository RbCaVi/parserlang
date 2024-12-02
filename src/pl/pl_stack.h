#ifndef PL_STACK_H
#define PL_STACK_H

#include "pv/pv.h"
#include "pl/pl_dump.h"

#include <stddef.h>
#include <stdint.h>

/*
 * The stack
 */

struct pl_stack_cells;

typedef struct {
  struct pl_stack_cells* cells; // pointer to the stack data, refcount, and size
  uint32_t top; // the number of filled cells
  uint32_t locals; // the stack index locals start at // also the location of retinfo
} pl_stack;

// stack indexes are positive up from locals or negative down from stack top
// 0 is the retinfo (not used)

pl_stack pl_stack_new();

pv pl_stack_get(pl_stack,int);
#define pl_stack_top(stack) pl_stack_get(stack,-1)
pl_stack pl_stack_popn(pl_stack, uint32_t);
#define pl_stack_pop(stack) pl_stack_popn(stack,1)
pl_stack pl_stack_set(pl_stack,pv,int);
pl_stack pl_stack_push(pl_stack,pv);

//pl_stack pl_stack_push_frame(pl_stack);
pl_stack pl_stack_split_frame(pl_stack, int, const char*); // used in call
const char *pl_stack_retaddr(pl_stack stack);
pl_stack pl_stack_pop_frame(pl_stack);

pl_stack pl_stack_ref(pl_stack);
void pl_stack_unref(pl_stack);

pv pl_stack_frames(pl_stack);

void pl_dump_stack_prefixed(pl_stack stack, pl_dump_prefix parts);
#define pl_dump_stack(val) {pl_dump_prefix __dump_prefix = pl_dump_new_prefix(); pl_dump_stack_prefixed(val, __dump_prefix); pl_dump_free_prefix(__dump_prefix);}

#endif
