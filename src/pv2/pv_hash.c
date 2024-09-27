#include "pv_hash.h"

static pv_hash_func pv_hash_table[256];

void pv_register_hash(pv_kind kind, pv_hash_func f) {
	pv_hash_table[kind] = f;
}

uint32_t pv_hash(pv val) {
	// get the function to use
	pv_hash_func f = pv_hash_table[pv_get_kind(pv_copy(val))];

	// the table is initialized to all 0, so a nonzero f has been initialized
	if (f != 0) {
		// use the function
		return f(val);
	}

	// default return value
	// what should this be anyway
	//abort(); // death
	return 0;
}