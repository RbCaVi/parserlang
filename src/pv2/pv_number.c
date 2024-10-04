#include "pv_number.h"
#include "pv_to_string.h"
#include "pv_equal.h"

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
	int l = snprintf(NULL, 0, "%f", num);
	assert(l >= 0);
	size_t length = (size_t)l;
	char* str = malloc(length + 1);
	snprintf(str, length + 1, "%f", num);
	return str;
}

static int pv_number_equal_self(pv val1, pv val2) {
	return pv_number_value(val1) == pv_number_value(val2);
}

void pv_number_install() {
	// be nice if there was a static assert but
	assert(sizeof(double) <= sizeof(struct pv_refcnt*));
	pv_register_kind(&number_kind, "number", NULL);
	pv_register_to_string(number_kind, pv_number_to_string);
	pv_register_equal_self(number_kind, pv_number_equal_self);
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