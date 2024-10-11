#include "pl_func.h"

#include "pv_private.h"

#include <stdlib.h>

pv_kind func_kind;

typedef struct {
  pv_refcnt refcnt;
  enum {
  	BYTECODE
  } type;
  union {
  	pl_bytecode bytecode;
  };
} pl_func_data;

static pl_func_data *pvp_func_get_data(pv val) {
	pl_func_data *f = (pl_func_data*)val.data;
	return f;
}

static void pl_func_free(pv val) {
	pl_func_data *f = pvp_func_get_data(val);
	switch(f->type) {
	case BYTECODE:
		pl_bytecode_free(f->bytecode);
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
	pl_func_data *f = pvp_func_get_data(fun);
	pv out;
	switch(f->type) {
	case BYTECODE:
		out = pl_call(pl, f->bytecode);
		break;
	}
	pv_free(fun);
	return out;
}