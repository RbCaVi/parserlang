#include "pv_equal.h"

static pv_equal_func pv_equal_table[256][256];

void pv_register_self_equal(pv_kind kind, pv_equal_func f) {
	pv_equal_table[kind][kind] = f;
}

void pv_register_equal(pv_kind kind1, pv_kind kind2, pv_equal_func f) {
	pv_equal_table[kind1][kind2] = f;
	pv_equal_table[kind2][kind1] = f;
}

int pv_equal(pv val1, pv val2) {
	// get the function to use
	pv_equal_func f = pv_equal_table[pv_get_kind(pv_copy(val1))][pv_get_kind(pv_copy(val2))];

	// the table is initialized to all 0, so a nonzero f has been initialized
	if (f != 0) {
		// use the function
		return f(val1, val2);
	}

	// default return value
	return pv_get_kind(val1) == pv_get_kind(val2);
}