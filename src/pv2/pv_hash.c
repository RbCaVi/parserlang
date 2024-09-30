#include "pv_hash.h"

static pv_hash_func pv_hash_table[256];

void pv_register_hash(pv_kind kind, pv_hash_func f) {
	pv_hash_table[kind] = f;
}

uint32_t pv_hash(pv val) {
	// get the function to use
	pv_hash_func f = pv_hash_table[pv_get_kind(val)];

	// the table is initialized to all 0, so a nonzero f has been initialized
	if (f != 0) {
		// use the function
		return f(val);
	}

	pv_free(val);

	// default return value
	// what should this be anyway
	//abort(); // death
	return 0;
}

/*
    cyrb53a beta (c) 2023 bryc (github.com/bryc)
    License: Public domain (or MIT if needed). Attribution appreciated.
    This is a work-in-progress, and changes to the algorithm are expected.
    The original cyrb53 has a slight mixing bias in the low bits of h1.
    This doesn't affect collision rate, but I want to try to improve it.
    This new version has preliminary improvements in avalanche behavior.
*/
// i (rbcavi) ported it to c (obviously)
// i also changed the output a bit
// (i have no knowledge of hash algorithms, i just picked this one because i found it on stack overflow)
uint32_t pvp_hash_data(unsigned char *data, uint32_t len) {
	// this algorithm assumes overflow is wrapped
  uint32_t h1 = 0xdeadbeef, h2 = 0x41c6ce57;
  for (uint32_t i = 0; i < len; i++) {
  	unsigned char c = (unsigned char)data[i];
    h1 = (h1 ^ c) * 0x85ebca77;
    h2 = (h2 ^ c) * 0xc2b2ae3d;
  }
  h1 ^= (h1 ^ (h2 >> 15)) * 0x735a2d97;
  h2 ^= (h2 ^ (h1 >> 15)) * 0xcaf649a9;
  h1 ^= h2 >> 16;
  h2 ^= h1 >> 16;
  return h1;
  //return 2097152 * (h2 >>> 0) + (h1 >>> 11);
}