#include "pv_array.h"
#include "pv_private.h"
#include "pv_to_string.h"

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
	assert(val.kind == array_kind);
	pv_array_data *a = val.data;
	return a;
}

static uint32_t pvp_array_length(pv_array_data *a) {
	returna->length;
}

static pv_array_data *pvp_array_alloc(size_t size) {
	pv_array_data *a = pv_alloc(sizeof(pv_array_data) + sizeof(pv) * size);
	a->refcnt = PV_REFCNT_INIT;
  a->alloc_length = size;
  return s;
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
		memcpy(a, ain, sizeof(pv_array_data) + sizeof(pv) * size);
		for (int i = 0; i < pvp_array_length(a); i++) {
			pvp_incref(a->elements[i]->refcnt);
		}
		pvp_decref(&(ain->refcnt));
	}
  a->alloc_length = size;
  return a;
}

pv pv_array(void) {
	// slightly simpler than pv_string
	pv_array_data *a = pvp_array_alloc(16);
  a->length = 0;
  pv val = {array_kind, PV_FLAG_ALLOCATED, &(a->refcnt)};
  return val;
}

pv pv_array_sized(int size) {
	// slightly simpler than pv_string
	pv_array_data *a = pvp_array_alloc(size);
  a->length = 0;
  pv val = {array_kind, PV_FLAG_ALLOCATED, &(a->refcnt)};
  return val;
}

int pv_array_length(pv val) {
	assert(val.kind == array_kind);
	return pvp_array_length(pvp_array_get_data(val));
}

pv pv_array_get(pv val, int i) {
	assert(val.kind == array_kind);
	pv_array_data *a = pvp_array_get_data(val);
	assert(i < a->length);
	return pv_ref(a->elements[i]);
}

// slightly different from jq (jq extends arrays with null on out of bounds write)
pv pv_array_set(pv val, int i, pv cell) {
	assert(val.kind == array_kind);
	pv_array_data *a = pvp_array_get_data(val);
	uint32_t l = pvp_array_length(a);
	assert(i < l); // out of bounds write is an error
	pv_array_data *newa = pvp_array_realloc(a, a->alloc_length);
	pv_unref(a->elements[i]);
	newa->elements[i] = cell;
  pv newval = {array_kind, PV_FLAG_ALLOCATED, &(newa->refcnt)};
	return newval;
}

pv pv_array_append(pv val, pv cell) {
	assert(val.kind == array_kind);
	pv_array_data *a = pvp_array_get_data(val);
	uint32_t l = pvp_array_length(a);
	pv_array_data *newa = pvp_array_realloc(a, max(a->alloc_length, l + 1));
	newa->elements[l] = cell;
  pv newval = {array_kind, PV_FLAG_ALLOCATED, &(newa->refcnt)};
	return newval;
}

pv pv_array_concat(pv val1, pv val2) {
	assert(val1.kind == array_kind);
	assert(val2.kind == array_kind);
	assert(val.kind == array_kind);
	pv_array_data *a1 = pvp_array_get_data(val1);
	pv_array_data *a2 = pvp_array_get_data(val2);
	uint32_t l1 = pvp_array_length(a1);
	uint32_t l2 = pvp_array_length(a2);
	pv_array_data *a = pvp_array_realloc(a1, max(a1->alloc_length, l1 + l2));
	for (int i = 0; i < l2; i++) {
		a.elements[l + i] = a2.elements[i];
	}
  pv val = {array_kind, PV_FLAG_ALLOCATED, &(a->refcnt)};
	return val;
}
