#ifndef PL_EXEC_H
#define PL_EXEC_H

#include "pl_stack.h"

typedef struct {
	pv *globals;
	pl_stack stack;
} pl_state;

typedef struct {
	const char *bytecode;
} pl_func;

pv pl_call(pl_state *state, pl_func f);

#endif