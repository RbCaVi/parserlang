#include "pv_singletons.h"
#include "pv_to_string.h"
#include "pv_equal.h"

#include <assert.h>
#include <stddef.h>
#include <string.h>

pv_kind null_kind;
pv_kind bool_kind;

// this is a great idea - (int){struct pv_refcnt*} is definitely worse
#define reinterpret_cast(t1, t2, v) ((union {t1 v1; t2 v2;}){.v1 = v}).v2

static char *pv_singleton_to_string(pv val) {
	if (pv_get_kind(val) == null_kind) {
		return strdup("NULL");
	}
	if (pv_bool_value(val)) {
		return strdup("TRUE");
	} else {
		return strdup("FALSE");
	}
}

static int pv_singleton_equal_self(pv val1, pv val2) {
	if (pv_get_kind(val1) == null_kind) {
		return 1;
	}
	return pv_bool_value(val1) == pv_bool_value(val2);
}

void pv_singletons_install() {
	// be nice if there was a static assert but
	assert(sizeof(int) <= sizeof(struct pv_refcnt*));
	pv_register_kind(&null_kind, "null", NULL);
	pv_register_kind(&bool_kind, "bool", NULL);
	pv_register_to_string(null_kind, pv_singleton_to_string);
	pv_register_to_string(bool_kind, pv_singleton_to_string);
	pv_register_equal_self(null_kind, pv_singleton_equal_self);
	pv_register_equal_self(bool_kind, pv_singleton_equal_self);
}

pv pv_null() {
	pv val = {null_kind, 0, reinterpret_cast(int, struct pv_refcnt*, 0)};
	return val;
}

pv pv_true() {
	pv val = {bool_kind, 0, reinterpret_cast(int, struct pv_refcnt*, 1)};
	return val;
}

pv pv_false() {
	pv val = {bool_kind, 0, reinterpret_cast(int, struct pv_refcnt*, -1)};
	return val;
}

pv pv_bool(int b) {
	pv val = {bool_kind, 0, reinterpret_cast(int, struct pv_refcnt*, b ? 1 : -1)};
	return val;
}

int pv_bool_value(pv val) {
	assert(val.kind == bool_kind);
	return (reinterpret_cast(struct pv_refcnt*, int, val.data) > 0) ? 1 : 0;
}