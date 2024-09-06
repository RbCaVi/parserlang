#include "pv_singletons.h"
#include "pv_to_string.h"

#include <assert.h>
#include <stddef.h>
#include <string.h>

pv_kind null_kind;
pv_kind bool_kind;

// this is a great idea - casting int to pointer is definitely worse

static struct pv_refcnt *cast_int_to_pointer(int val) {
	// illegal
	// arrest this man
	union {
		struct pv_refcnt *ptr;
		int val;
	} u;
	u.val = val;
	return u.ptr;
}

static int cast_pointer_to_int(struct pv_refcnt *ptr) {
	// illegal
	// arrest this man
	union {
		struct pv_refcnt *ptr;
		int val;
	} u;
	u.ptr = ptr;
	return u.val;
}

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

void pv_singletons_install() {
	// be nice if there was a static assert but
	assert(sizeof(int) <= sizeof(struct pv_refcnt*));
	pv_register_kind(&null_kind, "null", NULL);
	pv_register_kind(&bool_kind, "bool", NULL);
	pv_register_to_string(null_kind, pv_singleton_to_string);
	pv_register_to_string(bool_kind, pv_singleton_to_string);
}

pv pv_null() {
	pv val = {null_kind, 0, cast_int_to_pointer(0)};
	return val;
}

pv pv_true() {
	pv val = {bool_kind, 0, cast_int_to_pointer(1)};
	return val;
}

pv pv_false() {
	pv val = {bool_kind, 0, cast_int_to_pointer(-1)};
	return val;
}

pv pv_bool(int b) {
	pv val = {bool_kind, 0, cast_int_to_pointer(b ? 1 : -1)};
	return val;
}

int pv_bool_value(pv val) {
	assert(val.kind == bool_kind);
	return (cast_pointer_to_int(val.data) > 0) ? 1 : 0;
}