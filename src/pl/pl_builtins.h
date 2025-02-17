#ifndef PL_BUILTINS_H
#define PL_BUILTINS_H

#include "pl_func.h"
#include "pl_exec.h"

#define BUILTIN(pl_name, c_name) pv c_name(pl_state*);
#include "pl_builtins_data.h"
#undef BUILTIN

typedef struct {
	pl_func_type f;
	const char *name;
} pl_builtin;

extern pl_builtin pl_builtins[];

typedef enum {
#define BUILTIN(pl_name, c_name) c_name ## _id,
#include "pl_builtins_data.h"
#undef BUILTIN
} pl_builtin_ids;

#endif