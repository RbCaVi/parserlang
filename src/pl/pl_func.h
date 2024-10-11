#ifndef PL_FUNC_H
#define PL_FUNC_H

#include "pv.h"
#include "pl_exec.h"
//#include "pl_stack.h"

#include <stdint.h>

extern pv_kind func_kind;

void pv_func_install();

pv pv_func(pl_bytecode);
//pv pv_native_func(pv (*)(pl_stack));

pv pv_func_call(pv, pl_state*);

#endif