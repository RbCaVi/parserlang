#ifndef PL_DUMP_STATE_H
#define PL_DUMP_STATE_H

#include "pl_exec.h"
#include "pl_dump.h"

void pl_dump_state_prefixed(pl_state *state, pl_dump_prefix parts);
#define pl_dump_state(val) {pl_dump_prefix __dump_prefix = pl_dump_new_prefix(); pl_dump_state_prefixed(val, __dump_prefix); pl_dump_free_prefix(__dump_prefix);}

#endif