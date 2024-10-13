#include "pv_number.h"
#include "pv_to_string.h"
#include "pv_equal.h"

#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

pv_kind double_kind;
pv_kind int_kind;

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

static char *pv_double_to_string(pv val) {
	double num = pv_double_value(val);
	int l = snprintf(NULL, 0, "%f", num);
	assert(l >= 0);
	size_t length = (size_t)l;
	char* str = malloc(length + 1);
	snprintf(str, length + 1, "%f", num);
	return str;
}

static char *pv_int_to_string(pv val) {
	double num = pv_int_value(val);
	int l = snprintf(NULL, 0, "%f", num);
	assert(l >= 0);
	size_t length = (size_t)l;
	char* str = malloc(length + 1);
	snprintf(str, length + 1, "%f", num);
	return str;
}

static int pv_double_equal_self(pv val1, pv val2) {
	return pv_double_value(val1) == pv_double_value(val2);
}

static int pv_int_equal_self(pv val1, pv val2) {
	return pv_int_value(val1) == pv_int_value(val2);
}

void pv_number_install() {
	// be nice if there was a static assert but
	assert(sizeof(double) <= sizeof(struct pv_refcnt*));
	assert(sizeof(int) <= sizeof(struct pv_refcnt*));
	pv_register_kind(&double_kind, "double", NULL);
	pv_register_to_string(double_kind, pv_double_to_string);
	pv_register_equal_self(double_kind, pv_double_equal_self);
	pv_register_kind(&int_kind, "int", NULL);
	pv_register_to_string(int_kind, pv_int_to_string);
	pv_register_equal_self(int_kind, pv_int_equal_self);
}

pv pv_double(double num) {
	pv val = {double_kind, 0, cast_double_to_pointer(num)};
	return val;
}

pv pv_int(int num) {
	pv val = {int_kind, 0, cast_int_to_pointer(num)};
	return val;
}

double pv_double_value(pv val) {
	// don't have to do a decref because number isn't allocated
	assert(val.kind == double_kind);
	double num = cast_pointer_to_double(val.data);
	return num;
}

int pv_int_value(pv val) {
	// don't have to do a decref because number isn't allocated
	assert(val.kind == int_kind);
	int num = cast_pointer_to_int(val.data);
	return num;
}

double pv_number_value(pv val) {
	if (val.kind == int_kind) {
		return pv_int_value(val);
	} else {
		return pv_double_value(val);
	}
}

int pv_number_int_value(pv val) {
	if (val.kind == int_kind) {
		return pv_int_value(val);
	} else {
		return (int)pv_double_value(val); // truncate + wrap for absolutely huge numbers (> ~2 billion)
	}
}

int pv_is_integer(pv);
