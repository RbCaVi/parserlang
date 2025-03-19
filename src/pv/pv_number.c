#include "pv_number.h"
#include "pv_to_string.h"
#include "pv_equal.h"

#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

pv_kind double_kind;
pv_kind int_kind;

// illegal
// arrest this man
// this needs a test at some point
#define reinterpret_cast(t1, t2, v) ((union {t1 v1; t2 v2;}){.v1 = v}).v2

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
	int num = pv_int_value(val);
	int l = snprintf(NULL, 0, "%i", num);
	assert(l >= 0);
	size_t length = (size_t)l;
	char* str = malloc(length + 1);
	snprintf(str, length + 1, "%i", num);
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
	//assert(sizeof(double) <= sizeof(struct pv_refcnt*));
	//assert(sizeof(int) <= sizeof(struct pv_refcnt*));
	pv_register_kind(&double_kind, "double", NULL);
	pv_register_to_string(double_kind, pv_double_to_string);
	pv_register_equal_self(double_kind, pv_double_equal_self);
	pv_register_kind(&int_kind, "int", NULL);
	pv_register_to_string(int_kind, pv_int_to_string);
	pv_register_equal_self(int_kind, pv_int_equal_self);
}

pv pv_double(double num) {
	pv val = {double_kind, 0, reinterpret_cast(double, struct pv_refcnt*, num)};
	return val;
}

pv pv_int(int num) {
	pv val = {int_kind, 0, reinterpret_cast(int, struct pv_refcnt*, num)};
	return val;
}

double pv_double_value(pv val) {
	// don't have to do a decref because number isn't allocated
	assert(val.kind == double_kind);
	double num = reinterpret_cast(struct pv_refcnt*, double, val.data);
	return num;
}

int pv_int_value(pv val) {
	// don't have to do a decref because number isn't allocated
	assert(val.kind == int_kind);
	int num = reinterpret_cast(struct pv_refcnt*, int, val.data);
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

#define UOP(upper_name, lower_name, op) \
pv pv_number_ ## lower_name(pv v) { \
	if (pv_get_kind(v) == double_kind) { \
		double n = pv_number_value(v); \
		return pv_double(op n); \
	} else { \
		int n = pv_int_value(v); \
		return pv_int(op n); \
	} \
}
#define BOP(upper_name, lower_name, op, isdefault) \
isdefault( \
pv pv_number_ ## lower_name(pv v1, pv v2) { \
	if (pv_get_kind(v1) == double_kind || pv_get_kind(v2) == double_kind) { \
		double n1 = pv_number_value(v1); \
		double n2 = pv_number_value(v2); \
		return pv_double(n1 op n2); \
	} else { \
		int n1 = pv_int_value(v1); \
		int n2 = pv_int_value(v2); \
		return pv_int(n1 op n2); \
	} \
} \
)
#include "pv_number_ops_data.h"
#undef UOP
#undef BOP

pv pv_number_div(pv v1, pv v2) {
	double n1 = pv_number_value(v1);
	double n2 = pv_number_value(v2);
	return pv_double(n1 / n2);
}

pv pv_number_idiv(pv v1, pv v2) {
	double n1 = pv_number_value(v1);
	double n2 = pv_number_value(v2);
	return pv_int((int)(n1 / n2)); // what? round towards -infinity? couldn't be.
}

pv pv_number_mod(pv v1, pv v2) {
	if (pv_get_kind(v1) == double_kind || pv_get_kind(v2) == double_kind) {
		double n1 = pv_number_value(v1);
		double n2 = pv_number_value(v2);
		return pv_double(fmod(n1, n2));
	} else {
		int n1 = pv_int_value(v1);
		int n2 = pv_int_value(v2);
		return pv_int(n1 % n2);
	}
}