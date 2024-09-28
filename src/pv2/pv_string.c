#include "pv_string.h"
#include "pv_private.h"
#include "pv_to_string.h"
#include "pv_hash.h"
#include "pv_equal.h"

#include <string.h>
#include <stddef.h>
#include <assert.h>
#include <stdint.h>
#include <stdlib.h>

pv_kind string_kind;

// this struct was copied straight out of jq
typedef struct {
  pv_refcnt refcnt;
  uint32_t hash;
  // high 31 bits are length, low bit is a flag
  // indicating whether hash has been computed.
  uint32_t length_hashed;
  uint32_t alloc_length;
  char data[];
} pv_string_data;

static pv_string_data *pvp_string_get_data(pv val) {
	pv_string_data *s = (pv_string_data*)val.data;
	return s;
}

static void pv_string_free(pv val) {
	free(pvp_string_get_data(val));
}

static uint32_t pvp_string_length(pv_string_data *s) {
	return s->length_hashed >> 1;
}

static int pvp_string_hashed(pv_string_data *s) {
	return s->length_hashed & 1;
}

static void pvp_string_sethash(pv_string_data *s, uint32_t hash) {
	s->length_hashed |= 1; // set the hashed bit
	s->hash = hash;
}

static void pvp_string_unhash(pv_string_data *s) {
	s->length_hashed &= ~1U; // unset the hashed bit
}

static char *pv_string_to_string(pv val) {
	pv_string_data *s = pvp_string_get_data(val);
	uint32_t len = pvp_string_length(s);
	char *str = pv_alloc(len + 1);
	memcpy(str, s->data, len);
	pv_free(val);
	return str;
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
static uint32_t pv_string_hash(pv val) {
	pv_string_data *s = pvp_string_get_data(val);
	if (pvp_string_hashed(s)) {
		return s->hash;
	}
	uint32_t len = pvp_string_length(s);
	// this algorithm assumes overflow is wrapped
  uint32_t h1 = 0xdeadbeef, h2 = 0x41c6ce57;
  for (uint32_t i = 0; i < len; i++) {
  	unsigned char c = (unsigned char)s->data[i];
    h1 = (h1 ^ c) * 0x85ebca77;
    h2 = (h2 ^ c) * 0xc2b2ae3d;
  }
  h1 ^= (h1 ^ (h2 >> 15)) * 0x735a2d97;
  h2 ^= (h2 ^ (h1 >> 15)) * 0xcaf649a9;
  h1 ^= h2 >> 16;
  h2 ^= h1 >> 16;
  pvp_string_sethash(s, h1);
  return h1;
  //return 2097152 * (h2 >>> 0) + (h1 >>> 11);
}

int pv_string_equal_self(pv val1, pv val2) {
	pv_string_data *s1 = pvp_string_get_data(val1);
	pv_string_data *s2 = pvp_string_get_data(val2);
	uint32_t len1 = pvp_string_length(s1);
	uint32_t len2 = pvp_string_length(s2);
	int out;
	if (s1 == s2) {
		// that was easy
		out = 1;
	} else if (len1 != len2) {
		out = 0;
	} else if (s1)
	pv_free(val1);
	pv_free(val2);
	return out;
}

void pv_string_install() {
	pv_register_kind(&string_kind, "string", pv_string_free);
	pv_register_to_string(string_kind, pv_string_to_string);
	pv_register_hash(string_kind, pv_string_hash);
	pv_register_equal_self(string_kind, pv_string_equal_self);
}

static pv_string_data *pv_string_alloc(size_t size) {
	pv_string_data *s = pv_alloc(sizeof(pv_string_data) + size + 1);
	s->refcnt = PV_REFCNT_INIT;
  s->alloc_length = (uint32_t)size;
  return s;
}

pv pv_string(const char *str) {
	uint32_t len = (uint32_t)strlen(str);
	pv_string_data *s = pv_string_alloc(len * 2);
  memcpy(s->data, str, len);
  s->length_hashed = len << 1; // just assume that nobody will use a 2 gb string and cause overflow
  pv val = {string_kind, PV_FLAG_ALLOCATED, &(s->refcnt)};
  return val;
}

uint32_t pv_string_length(pv val) {
	assert(val.kind == string_kind);
	return pvp_string_length(pvp_string_get_data(val));
}

//uint32_t pv_string_hash(pv val) {
//	assert(val.kind == string_kind);
//	pv_string_data *s = pvp_string_get_data(val);
//	// uhhhh i have to implement this at some point
//}

pv pv_string_concat(pv val1, pv val2) {
	assert(val1.kind == string_kind);
	assert(val2.kind == string_kind);
	pv_string_data *s1 = pvp_string_get_data(val1);
	pv_string_data *s2 = pvp_string_get_data(val2);
	pv_string_data *s;
	pv val;
	uint32_t l1 = pvp_string_length(s1);
	uint32_t l2 = pvp_string_length(s2);
	if (pvp_refcnt_unshared(&(s1->refcnt)) && s1->alloc_length > l1 + l2) {
		memcpy(s1->data + l1, s2->data, l2);
		s = s1;
		val = val1;
	} else {
		// make a new string
		s = pv_string_alloc((l1 + l2) * 2);
		memcpy(s->data, s1->data, l1);
		memcpy(s->data + l1, s2->data, l2);
	  pv tval = {string_kind, PV_FLAG_ALLOCATED, &(s->refcnt)};
	  val = tval;
		pv_free(val1); // consumed by the algorithm
	}
	s->length_hashed = (l1 + l2) << 1;
	pv_free(val2);
	return val;
}