#include "pl_func.h"

#include "pv_private.h"
#include "pv_array.h"

#include <stdlib.h>
#include <assert.h>
#include <dlfcn.h>
#include <stdio.h>
#include <string.h>

pv_kind func_kind;

typedef struct {
		pv_refcnt refcnt;
		enum {
			BYTECODE,
			NATIVE,
		} type;
		union {
			pl_bytecode bytecode;
			struct {
				void *library;
				char *file;
				char *name;
				pl_func_type func;
			};
		};
		pv closedvars;
} pl_func_data;

static pl_func_data *plp_func_get_data(pv val) {
	pl_func_data *f = (pl_func_data*)val.data;
	return f;
}

static void pl_func_free(pv val) {
	pl_func_data *f = plp_func_get_data(val);
	switch (f->type) {
	case BYTECODE:
		pl_bytecode_free(f->bytecode);
		break;
	case NATIVE:
		if (f->library != NULL) {
			dlclose(f->library);
		}
		break;
	}
	pv_free(f->closedvars);
	free(f);
}

void pl_func_install() {
	pv_register_kind(&func_kind, "pl_func", pl_func_free);
}

pv pl_func(pl_bytecode bytecode) {
	pl_func_data *f = pv_alloc(sizeof(pl_func_data));
	f->refcnt = PV_REFCNT_INIT;
	f->type = BYTECODE;
	f->bytecode = bytecode;
	f->closedvars = pv_array();
	return (pv){func_kind, PV_FLAG_ALLOCATED, {&(f->refcnt)}};
}

pv pl_func_call(pv fun, pl_state *pl) {
	assert(fun.kind == func_kind);
	pl_func_data *f = plp_func_get_data(fun);
	pv out;
	switch (f->type) {
	case BYTECODE:
		pl->stack = pl_stack_push(pl->stack, pv_copy(fun));
		const char *saved_return = pl->code;
		pl->code = NULL;
		pl_state_set_call(pl, 0);
		out = pl_next(pl);
		pl->code = saved_return;
		break;
	case NATIVE:
		out = f->func(pl);
	}
	pv_free(fun);
	return out;
}

pl_bytecode pl_func_get_bytecode(pv fun) {
	assert(fun.kind == func_kind);
	pl_func_data *f = plp_func_get_data(fun);
	assert(f->type == BYTECODE);
	pl_bytecode bytecode = f->bytecode;
	pl_bytecode_incref(bytecode);
	pv_free(fun);
	return bytecode;
}

static pv plp_func_native(pl_func_type func, void *library, char *filename, char *funcname) {
	pl_func_data *f = pv_alloc(sizeof(pl_func_data));
	f->refcnt = PV_REFCNT_INIT;
	f->type = NATIVE;
	f->library = library;
	f->func = func;
	f->file = filename;
	f->name = funcname;
	f->closedvars = pv_array();
	return (pv){func_kind, PV_FLAG_ALLOCATED, {&(f->refcnt)}};
}

pv pl_func_from_symbol(char *filename, char *funcname) {
	dlerror(); // clear error
	void *library = dlopen(filename, RTLD_LAZY);
	char *error;
	if ((error = dlerror()) != NULL) {
		printf("dlerror open: %s\n", error);
		abort();
	}
	void *func = dlsym(library, funcname);
	if ((error = dlerror()) != NULL) {
		printf("dlerror sym: %s\n", error);
		abort();
	}
	return plp_func_native((pl_func_type)(func), library, filename, funcname);
}

pv pl_func_native(pl_func_type func) {
	return plp_func_native(func, NULL, NULL, NULL);
}

int pl_func_is_native(pv fun) {
	assert(fun.kind == func_kind);
	pl_func_data *f = plp_func_get_data(fun);
	pv_free(fun);
	return f->type == NATIVE;
}

pl_func_native_data pl_func_get_native(pv fun) {
	assert(fun.kind == func_kind);
	pl_func_data *f = plp_func_get_data(fun);
	assert(f->type == NATIVE);
	pl_func_native_data data = (pl_func_native_data){f->func, f->file, f->name};
	f->library = NULL;
	pv_free(fun);
	return data;
}

pl_stack pl_func_push_closed_vars(pv fun, pl_stack stack) {
	assert(fun.kind == func_kind);
	pl_func_data *f = plp_func_get_data(fun);
	pv_array_foreach(f->closedvars, i, v) {
		stack = pl_stack_push(stack, v);
	}
	pv_free(fun);
	return stack;
}

static pl_func_data *plp_func_move(pl_func_data *fin) {
	pl_func_data *f;
	if (pvp_refcnt_unshared(&(fin->refcnt))) {
		// only one copy
		// don't have to do anything
		f = fin;
	} else {
		// there is more then one copy of this pv, so i have to copy it
		f = pv_alloc(sizeof(pl_func_data));
		memcpy(f, fin, sizeof(pl_func_data));
		switch (f->type) {
		case BYTECODE:
			pl_bytecode_incref(f->bytecode);
			break;
		case NATIVE:
			dlerror(); // clear error
			void *library = dlopen(f->file, RTLD_LAZY); // basically incref the library
			char *error;
			if ((error = dlerror()) != NULL) {
				printf("dlerror open: %s\n", error);
				abort();
			}
			break;
		}
		pv_copy(f->closedvars);
		pvp_decref(&(fin->refcnt));
		f->refcnt = PV_REFCNT_INIT;
	}
  return f;
}

pv pl_func_add_closure_var(pv fun, pv val) {
	assert(fun.kind == func_kind);
	pl_func_data *f = plp_func_get_data(fun);
	f = plp_func_move(f);
	f->closedvars = pv_array_append(f->closedvars, val);
	return (pv){func_kind, PV_FLAG_ALLOCATED, {&(f->refcnt)}};
}