#ifndef PL_FUNC_H
#define PL_FUNC_H

#include "pv.h"
#include "pl_exec.h"
//#include "pl_stack.h"

extern pv_kind func_kind;

typedef pv (*pl_func_type)(pl_state*);

typedef struct {
	pl_func_type f;
	char *file;
	char *name;
} pl_func_native_data;

void pl_func_install();

pv pl_func(pl_bytecode);
pv pl_func_from_symbol(char*, char*);
pv pl_func_native(pl_func_type);

// call with no arguments
pv pl_func_call(pv, pl_state*);

int pl_func_is_native(pv);

pl_bytecode pl_func_get_bytecode(pv);
pl_func_native_data pl_func_get_native(pv);

pl_stack pl_func_push_closed_vars(pv, pl_stack);
pv pl_func_add_closure_var(pv, pv);

#endif