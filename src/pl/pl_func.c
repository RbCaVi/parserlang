#include "pl_func.h"

#include "pv_private.h"

#include <stdlib.h>

typedef struct {
  pv_refcnt refcnt;
  enum {
  	BYTECODE
  } type;
  union {
  	pl_bytecode bytecode;
  };
} pv_func_data;

static pv_func_data *pvp_func_get_data(pv val) {
	pv_func_data *f = (pv_func_data*)val.data;
	return f;
}

static void pv_func_free(pv val) {
	pv_func_data *f = pvp_func_get_data(val);
	switch(f->type) {
	case BYTECODE:
		pl_bytecode_free(f->bytecode);
		break;
	}
	free(f);
}

void pv_func_install() {
	pv_register_kind(&func_kind, "pv_func", pv_func_free);
}

pv pv_func(pl_bytecode bytecode) {
	pv_func_data *f = pv_alloc(sizeof(pv_func_data));
	f->refcnt = PV_REFCNT_INIT;
	f->type = BYTECODE;
	f->bytecode = bytecode;
	return (pv){func_kind, PV_FLAG_ALLOCATED, &(f->refcnt)};
}

pv pv_func_call(pv fun, pl_state *pl) {
	pv_func_data *f = pvp_func_get_data(fun);
	pv out;
	switch(f->type) {
	case BYTECODE:
		out = pl_call(pl, (pl_func){f->bytecode});
		break;
	}
	pv_free(fun);
	return out;
}