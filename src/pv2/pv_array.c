#include "pv_array.h"
#include "pv_private.h"
#include "pv_to_string.h"

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
	pv_array_data *s = val.data;
	return s;
}

static uint32_t pvp_array_length(pv_array_data *s) {
	return s->length;
}

static pv_array_data *pv_array_alloc(size_t size) {
	pv_array_data *s = pv_alloc(sizeof(pv_array_data) + sizeof(pv) * size);
	s->refcnt = PV_REFCNT_INIT;
  s->alloc_length = size;
  return s;
}

pv pv_array(void) {
	// slightly simpler than pv_string
	pv_array_data *a = pv_array_alloc(16);
  a->length = 0;
  pv val = {array_kind, PV_FLAG_ALLOCATED, &(a->refcnt)};
  return val;
}

pv pv_array_sized(int size) {
	// slightly simpler than pv_string
	pv_array_data *a = pv_array_alloc(size);
  a->length = 0;
  pv val = {array_kind, PV_FLAG_ALLOCATED, &(a->refcnt)};
  return val;
}

int pv_array_length(pv val) {
	return pvp_array_length(pvp_array_get_data(val));
}

pv pv_array_get(pv val, int i) {
	pv_array_data *a = pvp_array_get_data(val);
	return
}

pv pv_array_set(pv, int, pv) {
	return
}

pv pv_array_append(pv, pv) {
	return
}

pv pv_array_concat(pv, pv) {
	return
}
