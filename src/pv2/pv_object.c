#include "pv_object.h"
#include "pv_private.h"
#include "pv_to_string.h"
#include "pv_string.h"

#include <assert.h>

pv_kind object_kind;

// struct copied straight out of jq
struct object_slot {
  int next; /* next slot with same hash, for collisions */
  uint32_t hash;
  pv string;
  pv value;
};

typedef struct {
  pv_refcnt refcnt;
  int next_free;
  int last_free;
  int length;
  struct object_slot elements[];
} pvp_object_data;

void pv_object_install();

static pv_object_data *pvp_object_alloc(size_t size) {
	// size is the number of slots to use
	// must be a power of 2 for slightly simpler code
	assert(size > 0 && (size & (size - 1)) == 0);
	
	size_t nslots = size;
	size_t nbuckets = size * 2;
	pv_object_data *o = pv_alloc(sizeof(pv_object_data) + sizeof(object_slot) * nslots + sizeof(int) * nbuckets);
	o->refcnt = PV_REFCNT_INIT;
	o->next_free = 0;
	o->last_free = nslots - 1;
	o->length = 0;
	for (int i = 0; i < nslots; i++) {
		o->elements[i].next = i + 1; // the last one will point outside (but thats ok)
		o->elements[i].hash = 0;
		o->elements[i].string = pv_invalid();
		o->elements[i].value = pv_invalid();
	}
	int *buckets = (int*)(&o->elements[size]);
	for (int i = 0; i < nbuckets; i++) {
		buckets[i] = -1; // no.
	}
  return o;
}

pv pv_object(void) {
	pv_object_data *o = pvp_object_alloc(8);
  pv val = {object_kind, PV_FLAG_ALLOCATED, &(o->refcnt)};
  return val;
}

object_slot *pvp_object_get_slot(pvp_object_data *o, pv key) {
	uint32_t hash = pv_hash(key);
	
	uint32_t bucket = pvp_object_get_bucket(o, hash);
	int sloti = pvp_object_buckets(o)[bucket];
	object_slot *slot;
	
	while (sloti != -1) {
		slot = o->slots + sloti;
		if (slot->hash == hash && pv_equal(key, slot->string)) {
			return slot;
		}
		sloti = slot.next;
	}
	return NULL;
}

pv pv_object_get(pv obj, pv key) {
	assert(obj.kind == object_kind);
	
	pvp_object_data *o = pvp_object_get_data(obj);
	
	object_slot *slot = pvp_object_get_slot(o, key);
	
	pv_free(obj);
	pv_free(key);
	
	if (slot == NULL) {
		return pv_invalid();
	}
	
	return pv_copy(slot->value);
}

int pv_object_has(pv obj, pv key) {
	assert(obj.kind == object_kind);
	
	pvp_object_data *o = pvp_object_get_data(obj);
	
	int out = pvp_object_get_slot(o, key) != NULL;
	
	pv_free(obj);
	pv_free(key);
	
	return out;
}

pv pv_object_set(pv object, pv key, pv value);

pv pv_object_delete(pv object, pv key) {
	assert(obj.kind == object_kind);
	
	pvp_object_data *o = pvp_object_get_data(obj);
	
	object_slot *slot = pvp_object_get_slot(o, key);
	
	pv_free(obj);
	pv_free(key);
	
	if (slot == NULL) {
		return 0;
	}
	
	pv_free(slot->key);
	pv_free(slot->value);
	int sloti = slot - &o->slots;
	o->slots[o->last_free].next = sloti;
	o->last_free = sloti;
	o->size--;
	
	return 1;
}

int pv_object_length(pv object);

pv pv_object_merge(pv, pv);

pv pv_object_merge_recursive(pv, pv);

int pv_object_iter(pv);

int pv_object_iter_next(pv, int);

int pv_object_iter_valid(pv, int);

pv pv_object_iter_key(pv, int);

pv pv_object_iter_value(pv, int);