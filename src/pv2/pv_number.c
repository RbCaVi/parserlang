#include "pv_number.h"
#include "pv_to_string.h"

#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

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

static char *pv_number_to_string(pv val) {
	double num = pv_number_value(val);
	size_t length = snprintf(NULL, 0, "%f", num);
	char* str = malloc(length + 1);
	snprintf(str, length + 1, "%f", num);
	return str;
}

void pv_number_install() {
	// be nice if there was a static assert but
	assert(sizeof(double) <= sizeof(struct pv_refcnt*));
	pv_register_kind(&number_kind, "number", NULL);
	pv_register_to_string(number_kind, pv_number_to_string);
}

pv pv_number(double num) {
	pv val = {number_kind, 0, cast_double_to_pointer(num)};
	return val;
}

double pv_number_value(pv val) {
	// don't have to do a decref because number isn't allocated
	assert(val.kind == number_kind);
	double num = cast_pointer_to_double(val.data);
	return num;
}