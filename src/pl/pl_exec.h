#ifndef PL_EXEC_H
#define PL_EXEC_H

#include "pl_stack.h"
#include "pl_bytecode.h"

typedef struct pl_state {
  const char *code;
  struct pl_state *save; // the last save point - may be NULL - linked list
  struct pl_state *parent; // the state that called this generator - may be NULL for top level
	pv *globals;
	pl_stack stack;
} pl_state;

void pl_state_set_call(pl_state *state, int argc, const char *ret); // assuming f is already on the stack - ret is the return address
pv pl_next(pl_state *state);
pl_state *pl_state_new();
pl_state *pl_state_dup(pl_state *state);
void pl_state_free(pl_state *state);

#endif