#include "pv_number.h"

#include <assert.h>

// this needs a test at some point

static struct pv_refcnt *cast_double_to_pointer(double val) {
	// illegal
	// arrest this man
	assert(sizeof(double) == sizeof(struct pv_refcnt*))
	union {
		struct pv_refcnt *ptr; // only used for casting
		double val;
	} u;
	u.val = val;
	return u.ptr;
}

static double cast_pointer_to_double(struct pv_refcnt *ptr) {
	// illegal
	// arrest this man
	assert(sizeof(double) == sizeof(struct pv_refcnt*))
	union {
		struct pv_refcnt *ptr; // only used for casting
		double val;
	} u;
	u.ptr = ptr;
	return u.val;
}

void pv_number_install() {
	pv_register_kind(&number_kind, "number", NULL);
}

pv pv_number(double num) {
	pv val = {number_kind, 0, cast_number_data(num)};
	return val;
}

double pv_number_value(pv val) {
	assert(val.kind == number_kind);
	double num = cast_pointer_to_double(val.data);
	return num;
}