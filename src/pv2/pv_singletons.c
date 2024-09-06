#include "pv_singletons.h"

pv_kind null_kind;
pv_kind bool_kind;

// this is a great idea - casting int to pointer is definitely more illegal

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

void pv_singletons_install() {
	// be nice if there was a static assert but
	assert(sizeof(int) <= sizeof(struct pv_refcnt*));
	pv_register_kind(&null_kind, "null", NULL);
	pv_register_kind(&bool_kind, "bool", NULL);
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