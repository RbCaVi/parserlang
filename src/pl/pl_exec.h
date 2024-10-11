#ifndef PL_EXEC_H
#define PL_EXEC_H

#include "pl_stack.h"
#include "pl_bytecode.h"

typedef struct {
	pv *globals;
	pl_stack stack;
} pl_state;

pv pl_call(pl_state *state, pl_bytecode f);

#endif