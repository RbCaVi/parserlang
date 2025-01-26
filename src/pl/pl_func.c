#include "pl_func.h"

#include "pv_private.h"

#include <stdlib.h>
#include <assert.h>
#include <dlfcn.h>
#include <stdio.h>

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
	return (pv){func_kind, PV_FLAG_ALLOCATED, &(f->refcnt)};
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
	pv_free(fun);
	pl_bytecode_incref(bytecode);
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
	return (pv){func_kind, PV_FLAG_ALLOCATED, &(f->refcnt)};
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
