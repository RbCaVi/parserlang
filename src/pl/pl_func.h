#ifndef PL_FUNC_H
#define PL_FUNC_H

#include "pv.h"
#include "pl_exec.h"
//#include "pl_stack.h"

#include <stdint.h>

extern pv_kind func_kind;

void pl_func_install();

pv pl_func(pl_bytecode);
//pv pv_native_func(pv (*)(pl_stack));

pv pl_func_call(pv, pl_state*);

#endif