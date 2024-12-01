#ifndef PL_EXEC_H
#define PL_EXEC_H

#include "pl_stack.h"
#include "pl_bytecode.h"

typedef struct {
 uint32_t refcnt;
 const char *code;
 pl_state *parent; // may be NULL
	pv *globals;
	pl_stack stack;
} pl_state;

void pl_state_set_call(pl_state *state, int argc, const char *ret); // assuming f is already on the stack - ret is the return address
pv pl_next(pl_state *state);
pl_state *pl_state_copy(pl_state *state);
pl_state *pl_state_free(pl_state *state);

#endif