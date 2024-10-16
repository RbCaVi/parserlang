#include "pv_array.h"
#include "pv_private.h"
#include "pv_to_string.h"
#include "pv_equal.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#define max(a,b) ((a) > (b) ? (a) : (b))

pv_kind array_kind;

// this struct was copied straight out of jq
typedef struct {
  pv_refcnt refcnt;
  uint32_t length;
  uint32_t alloc_length;
  pv elements[];
} pv_array_data;

static pv_array_data *pvp_array_get_data(pv val) {
	pv_array_data *a = (pv_array_data*)val.data;
	return a;
}

static uint32_t pvp_array_length(pv_array_data *a) {
	return a->length;
}

static void pv_array_free(pv val) {
	pv_array_data *a = pvp_array_get_data(val);
	uint32_t l = pvp_array_length(a);
	for (uint32_t i = 0; i < l; i++) {
		pv_free(a->elements[i]);
	}
	free(a);
}

static char *pv_array_to_string(pv val) {
	pv_array_data *a = pvp_array_get_data(val);
	uint32_t l = pvp_array_length(a);

	uint32_t tlen = 1 + 2 * (l == 0 ? 0 : l - 1) + 1; // "[" + ", "... + "]"
	char **strs = pv_alloc(sizeof(char*) * l);
	uint32_t *lens = pv_alloc(sizeof(uint32_t) * l);

	for (uint32_t i = 0; i < l; i++) {
		char *str = pv_to_string(pv_copy(a->elements[i]));
		uint32_t len = (uint32_t)strlen(str);
		tlen += len;
		strs[i] = str;
		lens[i] = len;
	}

	char *str = pv_alloc(tlen + 1);
	str[0] = '[';
	uint32_t pos = 1;

	for (uint32_t i = 0; i < l; i++) {
		uint32_t len = lens[i];
		memcpy(str + pos, strs[i], len);
		free(strs[i]);
		pos += len;
		if (i < l - 1) {
			memcpy(str + pos, ", ", 2);
			pos += 2;
		}
	}
	str[pos] = ']';
	str[pos + 1] = '\0';

	free(strs);
	free(lens);

	pv_free(val);

	return str;
}

static int pv_array_equal_self(pv val1, pv val2) {
	pv_array_data *a1 = pvp_array_get_data(val1);
	pv_array_data *a2 = pvp_array_get_data(val2);
	uint32_t l1 = pvp_array_length(a1);
	uint32_t l2 = pvp_array_length(a2);
	if (l1 != l2) {
		return 0;
	}
	uint32_t l = l1; // l2 is equal
	int out = 1;
	for (uint32_t i = 0; i < l; i++) {
		if (!pv_equal(pv_copy(a1->elements[i]), pv_copy(a2->elements[i]))) {
			out = 0;
			break;
		}
	}
	pv_array_free(val1);
	pv_array_free(val2);
	return out;
}

void pv_array_install() {
	pv_register_kind(&array_kind, "array", pv_array_free);
	pv_register_to_string(array_kind, pv_array_to_string);
	pv_register_equal_self(array_kind, pv_array_equal_self);
}

static pv_array_data *pvp_array_alloc(size_t size) {
	pv_array_data *a = pv_alloc(sizeof(pv_array_data) + sizeof(pv) * size);
	a->refcnt = PV_REFCNT_INIT;
  a->alloc_length = (uint32_t)size;
  return a;
}

static pv_array_data *pvp_array_realloc(pv_array_data *ain, size_t size) {
	assert(size >= ain->length);
	pv_array_data *a;
	if (pvp_refcnt_unshared(&(ain->refcnt))) {
		// there is only one copy of this pv, so i can rebuild it with realloc
		// if (size <= ain->alloc_length) {
		// 	// don't need to reallocate if it would shorten the allocation
		// 	return ain;
		// }
		a = pv_realloc(ain, sizeof(pv_array_data) + sizeof(pv) * size);
	} else {
		// there is more then one copy of this pv, so i have to 
		a = pv_alloc(sizeof(pv_array_data) + sizeof(pv) * size);
		memcpy(a, ain, sizeof(pv_array_data) + sizeof(pv) * ain->length);
		for (uint32_t i = 0; i < pvp_array_length(a); i++) {
			pv_copy(a->elements[i]);
		}
		pvp_decref(&(ain->refcnt));
		a->refcnt = PV_REFCNT_INIT;
	}
  a->alloc_length = (uint32_t)size;
  return a;
}

pv pv_array(void) {
	// slightly simpler than pv_string
	pv_array_data *a = pvp_array_alloc(16);
  a->length = 0;
  pv val = {array_kind, PV_FLAG_ALLOCATED, &(a->refcnt)};
  return val;
}

pv pv_array_sized(uint32_t size) {
	// slightly simpler than pv_string
	pv_array_data *a = pvp_array_alloc(size);
  a->length = 0;
  pv val = {array_kind, PV_FLAG_ALLOCATED, &(a->refcnt)};
  return val;
}

uint32_t pv_array_length(pv val) {
	assert(val.kind == array_kind);
	uint32_t out = pvp_array_length(pvp_array_get_data(val));
	pv_free(val);
	return out;
}

int pvp_array_wrap_idx(int i, uint32_t l) {
	if (i < 0) {
		i += (int)l;
	}
	assert(i < (int)l);
	assert(i >= 0);
	return i;
}

pv pv_array_get(pv val, int i) {
	assert(val.kind == array_kind);
	pv_array_data *a = pvp_array_get_data(val);
	i = pvp_array_wrap_idx(i, pvp_array_length(a));
	pv out = pv_copy(a->elements[i]);
	pv_free(val);
	return out;
}

// slightly different from jq (jq extends arrays with null on out of bounds write)
pv pv_array_set(pv val, int i, pv cell) {
	assert(val.kind == array_kind);
	pv_array_data *a = pvp_array_get_data(val);
	uint32_t l = pvp_array_length(a);
	i = pvp_array_wrap_idx(i, l);
	pv_array_data *newa = pvp_array_realloc(a, a->alloc_length);
	pv_free(newa->elements[i]);
	newa->elements[i] = cell;
  pv newval = {array_kind, PV_FLAG_ALLOCATED, &(newa->refcnt)};
	return newval;
}

pv pv_array_append(pv val, pv cell) {
	assert(val.kind == array_kind);
	pv_array_data *a = pvp_array_get_data(val);
	uint32_t l = pvp_array_length(a);
	pv_array_data *newa = pvp_array_realloc(a, max(a->alloc_length, l + 1));
	newa->length = l + 1;
	newa->elements[l] = cell;
  pv newval = {array_kind, PV_FLAG_ALLOCATED, &(newa->refcnt)};
	return newval;
}

pv pv_array_concat(pv val1, pv val2) {
	assert(val1.kind == array_kind);
	assert(val2.kind == array_kind);
	pv_array_data *a1 = pvp_array_get_data(val1);
	pv_array_data *a2 = pvp_array_get_data(val2);
	uint32_t l1 = pvp_array_length(a1);
	uint32_t l2 = pvp_array_length(a2);
	pv_array_data *a = pvp_array_realloc(a1, max(a1->alloc_length, l1 + l2));
	a->length = l1 + l2;
	for (uint32_t i = 0; i < l2; i++) {
		a->elements[l1 + i] = pv_copy(a2->elements[i]);
	}
  pv val = {array_kind, PV_FLAG_ALLOCATED, &(a->refcnt)};
  pv_free(val2);
	return val;
}
