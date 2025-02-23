#ifndef PL_BUILTINS_H
#define PL_BUILTINS_H

#include "pl_func.h"
#include "pl_exec.h"

#define BUILTIN(name) pv pl_builtin_ ## name(pl_state*);
#include "pl_builtins_data.h"
#undef BUILTIN

typedef struct {
	pl_func_type f;
	const char *name;
} pl_builtin;

extern pl_builtin pl_builtins[];

typedef enum {
#define BUILTIN(name) pl_builtin_ ## name ## _id,
#include "pl_builtins_data.h"
#undef BUILTIN
} pl_builtin_ids;

#endif