#include "pv_number.h"

#include <assert.h>
#include <stddef.h>

pv_kind number_kind;

// this needs a test at some point

static struct pv_refcnt *cast_double_to_pointer(double val) {
	// illegal
	// arrest this man
	union {
		struct pv_refcnt *ptr;
		double val;
	} u;
	u.val = val;
	return u.ptr;
}

static double cast_pointer_to_double(struct pv_refcnt *ptr) {
	// illegal
	// arrest this man
	union {
		struct pv_refcnt *ptr;
		double val;
	} u;
	u.ptr = ptr;
	return u.val;
}

void pv_number_install() {
	// be nice if there was a static assert but
	assert(sizeof(double) <= sizeof(struct pv_refcnt*));
	pv_register_kind(&number_kind, "number", NULL);
}

pv pv_number(double num) {
	pv val = {number_kind, 0, cast_double_to_pointer(num)};
	return val;
}

double pv_number_value(pv val) {
	assert(val.kind == number_kind);
	double num = cast_pointer_to_double(val.data);
	return num;
}