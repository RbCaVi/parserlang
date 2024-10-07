#include "pv_object.h"
#include "pv_private.h"
#include "pv_to_string.h"
#include "pv_hash.h"
#include "pv_equal.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

pv_kind object_kind;

// struct copied straight out of jq
struct object_slot {
  int next; /* next slot with same hash, for collisions */
  uint32_t hash;
  pv key;
  pv value;
};

typedef struct {
  pv_refcnt refcnt;
  uint32_t next_free;
  uint32_t last_free;
  uint32_t length;
  uint32_t alloc_size;
  struct object_slot elements[];
} pvp_object_data;

static pvp_object_data *pvp_object_get_data(pv val) {
	pvp_object_data *o = (pvp_object_data*)val.data;
	return o;
}

static int *pvp_object_buckets(pvp_object_data *o) {
	return (int*)(&o->elements[o->alloc_size]);
}

static pvp_object_data *pvp_object_alloc(uint32_t size) {
	// size is the number of slots to use
	// must be a power of 2 for simpler (and probably faster) code
	assert(size > 0 && (size & (size - 1)) == 0);
	
	uint32_t nslots = size;
	uint32_t nbuckets = size * 2;
	pvp_object_data *o = pv_alloc(sizeof(pvp_object_data) + sizeof(struct object_slot) * nslots + sizeof(int) * nbuckets);
	o->refcnt = PV_REFCNT_INIT;
	o->next_free = 0;
	o->last_free = nslots - 1;
	o->length = 0;
	o->alloc_size = size;
	for (int i = 0; i < (int)nslots; i++) {
		o->elements[i].next = i + 1; // the last one will point outside (but thats ok)
		o->elements[i].hash = 0;
		o->elements[i].key = pv_invalid();
		o->elements[i].value = pv_invalid();
	}
	int *buckets = pvp_object_buckets(o);
	for (uint32_t i = 0; i < nbuckets; i++) {
		buckets[i] = -1; // no.
	}
  return o;
}

static uint32_t pvp_object_get_bucket(pvp_object_data *o, uint32_t hash) {
	// o->alloc_size is a power of 2 (for reasons)
	// this means hash % o->alloc_size can be replaced with hash & (o->alloc_size - 1)
	return hash & (o->alloc_size - 1);
}

static uint32_t round_up_next_pow2(uint32_t v) {
	v--;
	v |= v >> 1;
	v |= v >> 2;
	v |= v >> 4;
	v |= v >> 8;
	v |= v >> 16;
	v++;
	return v;
}

static pvp_object_data *pvp_object_realloc(pvp_object_data *oin, uint32_t size) {
	assert(size >= oin->length);

	uint32_t newsize = round_up_next_pow2(size + 1); // have to have enough space for one more

	pvp_object_data *o = pvp_object_alloc(newsize);

	int *bucketsin = pvp_object_buckets(oin);
	int *buckets = pvp_object_buckets(o);
	for (uint32_t i = 0; i < newsize * 2; i++) {
		if (bucketsin[i] == -1) {
			continue;
		}
		struct object_slot *slot;
		int sloti;
		for ((sloti = bucketsin[i]), (slot = &(oin->elements[sloti])); sloti !=-1; (sloti = (int)slot->next), (slot = &(oin->elements[sloti]))) {
			uint32_t bucket = pvp_object_get_bucket(o, slot->hash);

			uint32_t newsloti = o->next_free;
			o->next_free = (uint32_t)o->elements[newsloti].next;

			o->elements[newsloti].next = buckets[bucket];
			buckets[bucket] = (int)newsloti;

			o->elements[newsloti].hash = slot->hash;
			o->elements[newsloti].key = slot->key;
			o->elements[newsloti].value = slot->value;
		}
	}

	if (pvp_refcnt_unshared(&(oin->refcnt))) {
		free(oin); // nobody else needs this, right?
	} else {
		pvp_decref(&(oin->refcnt));
	}

  return o;
}

static void pv_object_free(pv obj) {
	pvp_object_data *o = pvp_object_get_data(obj);

	int *buckets = pvp_object_buckets(o);
	for (uint32_t i = 0; i < o->alloc_size * 2; i++) {
		if (buckets[i] == -1) {
			continue;
		}
		struct object_slot *slot;
		int sloti;
		for ((sloti = buckets[i]), (slot = &(o->elements[sloti])); sloti !=-1; (sloti = (int)slot->next), (slot = &(o->elements[sloti]))) {
			pv_free(slot->key);
			pv_free(slot->value);
		}
	}
}

static char *pv_object_to_string(pv val) {
	uint32_t l = pv_object_length(pv_copy(val));

	uint32_t tlen = 1 + 2 * l + 2 * (l - 1) + 1; // "{" + ": " + ", "... + "}"
	char **kstrs = pv_alloc(sizeof(char*) * l);
	uint32_t *klens = pv_alloc(sizeof(uint32_t) * l);
	char **vstrs = pv_alloc(sizeof(char*) * l);
	uint32_t *vlens = pv_alloc(sizeof(uint32_t) * l);

	uint32_t i;
	int iter;

	for (i = 0, iter = pv_object_iter(pv_copy(val)); i < l && pv_object_iter_valid(pv_copy(val), iter); i++, iter = pv_object_iter_next(pv_copy(val), iter)) {
		char *kstr = pv_to_string(pv_object_iter_key(pv_copy(val), iter));
		char *vstr = pv_to_string(pv_object_iter_value(pv_copy(val), iter));
		uint32_t klen = (uint32_t)strlen(kstr);
		uint32_t vlen = (uint32_t)strlen(vstr);
		tlen += klen;
		tlen += vlen;
		kstrs[i] = kstr;
		klens[i] = klen;
		vstrs[i] = vstr;
		vlens[i] = vlen;
	}

	char *str = pv_alloc(tlen + 1);
	str[0] = '{';
	uint32_t pos = 1;

	for (i = 0; i < l; i++) {
		uint32_t klen = klens[i];
		memcpy(str + pos, kstrs[i], klen);
		pos += klen;
		memcpy(str + pos, ": ", 2);
		pos += 2;
		uint32_t vlen = vlens[i];
		memcpy(str + pos, vstrs[i], vlen);
		pos += klen;
		if (i < l - 1) {
			memcpy(str + pos, ", ", 2);
			pos += 2;
		}
	}
	str[pos] = '}';
	str[pos + 1] = '\0';

	pv_free(val);
	return str;
}

void pv_object_install() {
	pv_register_kind(&object_kind, "object", pv_object_free);
	pv_register_to_string(object_kind, pv_object_to_string);
}

pv pv_object(void) {
	pvp_object_data *o = pvp_object_alloc(8);
  pv val = {object_kind, PV_FLAG_ALLOCATED, &(o->refcnt)};
  return val;
}

struct object_slot *pvp_object_get_slot(pvp_object_data *o, uint32_t hash, pv key) {
	uint32_t bucket = pvp_object_get_bucket(o, hash);
	int sloti = pvp_object_buckets(o)[bucket];
	struct object_slot *slot;
	
	while (sloti != -1) {
		slot = o->elements + sloti;
		if (slot->hash == hash && pv_equal(pv_copy(key), pv_copy(slot->key))) {
			pv_free(key);
			return slot;
		}
		sloti = (int)slot->next;
	}
	pv_free(key);
	return NULL;
}

pv pv_object_get(pv obj, pv key) {
	assert(obj.kind == object_kind);
	
	pvp_object_data *o = pvp_object_get_data(obj);
	
	struct object_slot *slot = pvp_object_get_slot(o, pv_hash(pv_copy(key)), key);
	
	if (slot == NULL) {
		pv_free(obj);
		return pv_invalid();
	}
	
	pv val = pv_copy(slot->value);
	
	pv_free(obj);

	return val;
}

int pv_object_has(pv obj, pv key) {
	assert(obj.kind == object_kind);
	
	pvp_object_data *o = pvp_object_get_data(obj);
	
	int out = pvp_object_get_slot(o, pv_hash(pv_copy(key)), key) != NULL;
	
	pv_free(obj);
	
	return out;
}

pv pv_object_set(pv obj, pv key, pv value) {
	assert(obj.kind == object_kind);
	
	pvp_object_data *o = pvp_object_get_data(obj);

	o = pvp_object_realloc(o, o->length + 1); // simpler than uh

	uint32_t hash = pv_hash(pv_copy(key));
	
	struct object_slot *slot = pvp_object_get_slot(o, hash, pv_copy(key));
	
	if (slot == NULL) {
		// use next_free
		// insert before slots[bucket] (in the linked list structure)
		uint32_t bucket = pvp_object_get_bucket(o, hash);

		uint32_t newsloti = o->next_free;
		o->next_free = (uint32_t)o->elements[newsloti].next;

		o->elements[newsloti].next = pvp_object_buckets(o)[bucket];
		pvp_object_buckets(o)[bucket] = (int)newsloti;

		o->elements[newsloti].hash = hash;
		o->elements[newsloti].key = key;
		o->elements[newsloti].value = value;

		o->length++;
	} else {
		// replace the value (it's as shrimple as that)
		// and free the key
		pv_free(slot->value);
		slot->value = value;
		pv_free(key);
	}
  pv val = {object_kind, PV_FLAG_ALLOCATED, &(o->refcnt)};
  return val;
}

pv pv_object_delete(pv obj, pv key) {
	assert(obj.kind == object_kind);
	
	pvp_object_data *o = pvp_object_get_data(obj);

	o = pvp_object_realloc(o, o->length - 1); // simpler than uh

	uint32_t hash = pv_hash(pv_copy(key));
	uint32_t bucket = pvp_object_get_bucket(o, hash);
	int *prevnext = &(pvp_object_buckets(o)[bucket]);
	int sloti = *prevnext;
	struct object_slot *slot;
	
	while (sloti != -1) {
		slot = o->elements + sloti;
		if (slot->hash == hash && pv_equal(pv_copy(key), pv_copy(slot->key))) {
			break;
		}
		prevnext = &(slot->next);
		sloti = *prevnext;
	}
	pv_free(key);
	if (sloti == -1) { // 404 - slot not found
		return obj;
	}
	
	// probably should reset the slot for no double free / writing unallocated memory shenanigans in pv_free()
	pv_free(slot->key);
	pv_free(slot->value);
	*prevnext = slot->next;
	sloti = (int)(slot - o->elements); // if it doesn't fit into an int, you probably have a problem
	o->elements[o->last_free].next = sloti;
	o->last_free = (uint32_t)sloti; // it it's negative, that's a problem too
	o->length--;

	pv newobj = {object_kind, PV_FLAG_ALLOCATED, &(o->refcnt)};
	
	return newobj;
}

uint32_t pv_object_length(pv obj) {
	assert(obj.kind == object_kind);
	
	pvp_object_data *o = pvp_object_get_data(obj);

	uint32_t l = o->length;

	pv_free(obj);

	return l;
}

pv pv_object_merge(pv, pv);

pv pv_object_merge_recursive(pv, pv);

int pv_object_iter(pv obj) {
	assert(obj.kind == object_kind);

	pvp_object_data *o = pvp_object_get_data(obj);

	uint32_t l = o->length;

	if (l == 0) {
		return -1;
	}

	int *buckets = pvp_object_buckets(o);
	for (uint32_t i = 0; i < o->alloc_size * 2; i++) {
		if (buckets[i] != -1) {
			pv_free(obj);
			return buckets[i];
		}
	}

	pv_free(obj);
	return -1; // just in case
}

// walk each linked list until the end and switch to the next one
int pv_object_iter_next(pv obj, int iter) {
	assert(obj.kind == object_kind);

	pvp_object_data *o = pvp_object_get_data(obj);

	if (o->elements[iter].next == -1) {
		int *buckets = pvp_object_buckets(o);
		for (uint32_t i = pvp_object_get_bucket(o, o->elements[iter].hash); i < o->alloc_size * 2; i++) {
			if (buckets[i] != -1) {
				pv_free(obj);
				return buckets[i];
			}
		}
		return -1;
	}

	int out = (int)o->elements[iter].next;

	pv_free(obj);

	return out;
}

int pv_object_iter_valid(pv obj, int iter) {
	assert(obj.kind == object_kind);

	pv_free(obj);

	return iter != -1;
}

pv pv_object_iter_key(pv obj, int iter) {
	assert(obj.kind == object_kind);
	assert(iter != -1);

	pvp_object_data *o = pvp_object_get_data(obj);

	pv val = pv_copy(o->elements[iter].key);

	pv_free(obj);

	return val;
}

pv pv_object_iter_value(pv obj, int iter) {
	assert(obj.kind == object_kind);
	assert(iter != -1);

	pvp_object_data *o = pvp_object_get_data(obj);

	pv val = pv_copy(o->elements[iter].value);

	pv_free(obj);

	return val;
}