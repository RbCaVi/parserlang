#include "pv_array.h"
#include "pv_private.h"
#include "pv_to_string.h"

pv_kind string_kind;

// this struct was copied straight out of jq
typedef struct {
  pv_refcnt refcnt;
  uint32_t length;
  uint32_t alloc_length;
  pv elements[];
} pv_array_data;

static pv_string_data *pv_string_alloc(size_t size) {
	pv_string_data *s = pv_alloc(sizeof(pv_string_data) + size + 1);
	s->refcnt = PV_REFCNT_INIT;
  s->alloc_length = size;
  return s;
}

pv pv_array(void) {
	pv_array_data *a = pv_array_alloc(16);
  a->length = len;
  pv val = {array_kind, PV_FLAG_ALLOCATED, &(a->refcnt)};
  return val;
}

int pv_array_length(pv) {
	return
}

pv pv_array_get(pv, int) {
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
