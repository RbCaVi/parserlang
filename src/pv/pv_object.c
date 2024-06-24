#include "pv_object.h"

#include "pv_invalid.h"
#include "pv_constants.h"
#include "pv_alloc.h"
#include "pvp.h"

#include <string.h>
#include <assert.h>

/*
 * Objects (internal helpers)
 */

#define PVP_FLAGS_OBJECT  PVP_MAKE_FLAGS(PV_KIND_OBJECT, PVP_PAYLOAD_ALLOCATED)

struct object_slot {
  int next; /* next slot with same hash, for collisions */
  uint32_t hash;
  pv string;
  pv value;
};

typedef struct {
  pv_refcnt refcnt;
  int next_free;
  struct object_slot elements[];
} pvp_object;


/* warning: nontrivial justification of alignment */
static pv pvp_object_new(int size) {
  // Allocates an object of (size) slots and (size*2) hash buckets.

  // size must be a power of two
  assert(size > 0 && (size & (size - 1)) == 0);

  pvp_object* obj = pv_mem_alloc(sizeof(pvp_object) +
                                 sizeof(struct object_slot) * size +
                                 sizeof(int) * (size * 2));
  obj->refcnt.count = 1;
  for (int i=0; i<size; i++) {
    obj->elements[i].next = i - 1;
    obj->elements[i].string = PV_NULL;
    obj->elements[i].hash = 0;
    obj->elements[i].value = PV_NULL;
  }
  obj->next_free = 0;
  int* hashbuckets = (int*)(&obj->elements[size]);
  for (int i=0; i<size*2; i++) {
    hashbuckets[i] = -1;
  }
  pv r = {PVP_FLAGS_OBJECT, 0, 0, size, {&obj->refcnt}};
  return r;
}

static pvp_object* pvp_object_ptr(pv o) {
  assert(PVP_HAS_KIND(o, PV_KIND_OBJECT));
  return (pvp_object*)o.u.ptr;
}

static uint32_t pvp_object_mask(pv o) {
  assert(PVP_HAS_KIND(o, PV_KIND_OBJECT));
  return (o.size * 2) - 1;
}

static int pvp_object_size(pv o) {
  assert(PVP_HAS_KIND(o, PV_KIND_OBJECT));
  return o.size;
}

static int* pvp_object_buckets(pv o) {
  return (int*)(&pvp_object_ptr(o)->elements[o.size]);
}

static int* pvp_object_find_bucket(pv object, pv key) {
  return pvp_object_buckets(object) + (pvp_object_mask(object) & pvp_string_hash(key));
}

static struct object_slot* pvp_object_get_slot(pv object, int slot) {
  assert(slot == -1 || (slot >= 0 && slot < pvp_object_size(object)));
  if (slot == -1) return 0;
  else return &pvp_object_ptr(object)->elements[slot];
}

static struct object_slot* pvp_object_next_slot(pv object, struct object_slot* slot) {
  return pvp_object_get_slot(object, slot->next);
}

static struct object_slot* pvp_object_find_slot(pv object, pv keystr, int* bucket) {
  uint32_t hash = pvp_string_hash(keystr);
  for (struct object_slot* curr = pvp_object_get_slot(object, *bucket);
       curr;
       curr = pvp_object_next_slot(object, curr)) {
    if (curr->hash == hash && pvp_string_equal(keystr, curr->string)) {
      return curr;
    }
  }
  return 0;
}

static struct object_slot* pvp_object_add_slot(pv object, pv key, int* bucket) {
  pvp_object* o = pvp_object_ptr(object);
  int newslot_idx = o->next_free;
  if (newslot_idx == pvp_object_size(object)) return 0;
  struct object_slot* newslot = pvp_object_get_slot(object, newslot_idx);
  o->next_free++;
  newslot->next = *bucket;
  *bucket = newslot_idx;
  newslot->hash = pvp_string_hash(key);
  newslot->string = key;
  return newslot;
}

static pv* pvp_object_read(pv object, pv key) {
  assert(PVP_HAS_KIND(key, PV_KIND_STRING));
  int* bucket = pvp_object_find_bucket(object, key);
  struct object_slot* slot = pvp_object_find_slot(object, key, bucket);
  if (slot == 0) return 0;
  else return &slot->value;
}

void pvp_object_free(pv o) {
  assert(PVP_HAS_KIND(o, PV_KIND_OBJECT));
  if (pvp_refcnt_dec(o.u.ptr)) {
    for (int i=0; i<pvp_object_size(o); i++) {
      struct object_slot* slot = pvp_object_get_slot(o, i);
      if (pv_get_kind(slot->string) != PV_KIND_NULL) {
        pvp_string_free(slot->string);
        pv_unref(slot->value);
      }
    }
    pv_mem_free(pvp_object_ptr(o));
  }
}

static pv pvp_object_rehash(pv object) {
  assert(PVP_HAS_KIND(object, PV_KIND_OBJECT));
  assert(pvp_refcnt_unshared(object.u.ptr));
  int size = pvp_object_size(object);
  pv new_object = pvp_object_new(size * 2);
  for (int i=0; i<size; i++) {
    struct object_slot* slot = pvp_object_get_slot(object, i);
    if (pv_get_kind(slot->string) == PV_KIND_NULL) continue;
    int* new_bucket = pvp_object_find_bucket(new_object, slot->string);
    assert(!pvp_object_find_slot(new_object, slot->string, new_bucket));
    struct object_slot* new_slot = pvp_object_add_slot(new_object, slot->string, new_bucket);
    assert(new_slot);
    new_slot->value = slot->value;
  }
  // references are transported, just drop the old table
  pv_mem_free(pvp_object_ptr(object));
  return new_object;
}

static pv pvp_object_unshare(pv object) {
  assert(PVP_HAS_KIND(object, PV_KIND_OBJECT));
  if (pvp_refcnt_unshared(object.u.ptr))
    return object;

  pv new_object = pvp_object_new(pvp_object_size(object));
  pvp_object_ptr(new_object)->next_free = pvp_object_ptr(object)->next_free;
  for (int i=0; i<pvp_object_size(new_object); i++) {
    struct object_slot* old_slot = pvp_object_get_slot(object, i);
    struct object_slot* new_slot = pvp_object_get_slot(new_object, i);
    *new_slot = *old_slot;
    if (pv_get_kind(old_slot->string) != PV_KIND_NULL) {
      new_slot->string = pv_ref(old_slot->string);
      new_slot->value = pv_ref(old_slot->value);
    }
  }

  int* old_buckets = pvp_object_buckets(object);
  int* new_buckets = pvp_object_buckets(new_object);
  memcpy(new_buckets, old_buckets, sizeof(int) * pvp_object_size(new_object)*2);

  pvp_object_free(object);
  assert(pvp_refcnt_unshared(new_object.u.ptr));
  return new_object;
}

static pv* pvp_object_write(pv* object, pv key) {
  *object = pvp_object_unshare(*object);
  int* bucket = pvp_object_find_bucket(*object, key);
  struct object_slot* slot = pvp_object_find_slot(*object, key, bucket);
  if (slot) {
    // already has the key
    pvp_string_free(key);
    return &slot->value;
  }
  slot = pvp_object_add_slot(*object, key, bucket);
  if (slot) {
    slot->value = pv_invalid();
  } else {
    *object = pvp_object_rehash(*object);
    bucket = pvp_object_find_bucket(*object, key);
    assert(!pvp_object_find_slot(*object, key, bucket));
    slot = pvp_object_add_slot(*object, key, bucket);
    assert(slot);
    slot->value = pv_invalid();
  }
  return &slot->value;
}

static int pvp_object_delete(pv* object, pv key) {
  assert(PVP_HAS_KIND(key, PV_KIND_STRING));
  *object = pvp_object_unshare(*object);
  int* bucket = pvp_object_find_bucket(*object, key);
  int* prev_ptr = bucket;
  uint32_t hash = pvp_string_hash(key);
  for (struct object_slot* curr = pvp_object_get_slot(*object, *bucket);
       curr;
       curr = pvp_object_next_slot(*object, curr)) {
    if (hash == curr->hash && pvp_string_equal(key, curr->string)) {
      *prev_ptr = curr->next;
      pvp_string_free(curr->string);
      curr->string = PV_NULL;
      pv_unref(curr->value);
      return 1;
    }
    prev_ptr = &curr->next;
  }
  return 0;
}

static int pvp_object_length(pv object) {
  int n = 0;
  for (int i=0; i<pvp_object_size(object); i++) {
    struct object_slot* slot = pvp_object_get_slot(object, i);
    if (pv_get_kind(slot->string) != PV_KIND_NULL) n++;
  }
  return n;
}
/*
static int pvp_object_equal(pv o1, pv o2) {
  int len2 = pvp_object_length(o2);
  int len1 = 0;
  for (int i=0; i<pvp_object_size(o1); i++) {
    struct object_slot* slot = pvp_object_get_slot(o1, i);
    if (pv_get_kind(slot->string) == PV_KIND_NULL) continue;
    pv* slot2 = pvp_object_read(o2, slot->string);
    if (!slot2) return 0;
    // FIXME: do less refcounting here
    if (!pv_equal(pv_ref(slot->value), pv_ref(*slot2))) return 0;
    len1++;
  }
  return len1 == len2;
}
*//*
static int pvp_object_contains(pv a, pv b) {
  assert(PVP_HAS_KIND(a, PV_KIND_OBJECT));
  assert(PVP_HAS_KIND(b, PV_KIND_OBJECT));
  int r = 1;

  pv_object_foreach(b, key, b_val) {
    pv a_val = pv_object_get(pv_ref(a), key);

    r = pv_contains(a_val, b_val);

    if (!r) break;
  }
  return r;
}
*/
/*
 * Objects (public interface)
 */
#define DEFAULT_OBJECT_SIZE 8
pv pv_object() {
  return pvp_object_new(8);
}

pv pv_object_get(pv object, pv key) {
  assert(PVP_HAS_KIND(object, PV_KIND_OBJECT));
  assert(PVP_HAS_KIND(key, PV_KIND_STRING));
  pv* slot = pvp_object_read(object, key);
  pv val;
  if (slot) {
    val = pv_ref(*slot);
  } else {
    val = pv_invalid();
  }
  pv_unref(object);
  pv_unref(key);
  return val;
}

int pv_object_has(pv object, pv key) {
  assert(PVP_HAS_KIND(object, PV_KIND_OBJECT));
  assert(PVP_HAS_KIND(key, PV_KIND_STRING));
  pv* slot = pvp_object_read(object, key);
  int res = slot ? 1 : 0;
  pv_unref(object);
  pv_unref(key);
  return res;
}

pv pv_object_set(pv object, pv key, pv value) {
  assert(PVP_HAS_KIND(object, PV_KIND_OBJECT));
  assert(PVP_HAS_KIND(key, PV_KIND_STRING));
  // copy/free of object, key, value coalesced
  pv* slot = pvp_object_write(&object, key);
  pv_unref(*slot);
  *slot = value;
  return object;
}

pv pv_object_delete(pv object, pv key) {
  assert(PVP_HAS_KIND(object, PV_KIND_OBJECT));
  assert(PVP_HAS_KIND(key, PV_KIND_STRING));
  pvp_object_delete(&object, key);
  pv_unref(key);
  return object;
}

int pv_object_length(pv object) {
  assert(PVP_HAS_KIND(object, PV_KIND_OBJECT));
  int n = pvp_object_length(object);
  pv_unref(object);
  return n;
}

pv pv_object_merge(pv a, pv b) {
  assert(PVP_HAS_KIND(a, PV_KIND_OBJECT));
  pv_object_foreach(b, k, v) {
    a = pv_object_set(a, k, v);
  }
  pv_unref(b);
  return a;
}

pv pv_object_merge_recursive(pv a, pv b) {
  assert(PVP_HAS_KIND(a, PV_KIND_OBJECT));
  assert(PVP_HAS_KIND(b, PV_KIND_OBJECT));

  pv_object_foreach(b, k, v) {
    pv elem = pv_object_get(pv_ref(a), pv_ref(k));
    if (pv_is_valid(elem) &&
        PVP_HAS_KIND(elem, PV_KIND_OBJECT) &&
        PVP_HAS_KIND(v, PV_KIND_OBJECT)) {
      a = pv_object_set(a, k, pv_object_merge_recursive(elem, v));
    } else {
      pv_unref(elem);
      a = pv_object_set(a, k, v);
    }
  }
  pv_unref(b);
  return a;
}

/*
 * Object iteration (internal helpers)
 */

enum { ITER_FINISHED = -2 };

int pv_object_iter_valid(pv object, int i) {
  return i != ITER_FINISHED;
}

int pv_object_iter(pv object) {
  assert(PVP_HAS_KIND(object, PV_KIND_OBJECT));
  return pv_object_iter_next(object, -1);
}

int pv_object_iter_next(pv object, int iter) {
  assert(PVP_HAS_KIND(object, PV_KIND_OBJECT));
  assert(iter != ITER_FINISHED);
  struct object_slot* slot;
  do {
    iter++;
    if (iter >= pvp_object_size(object))
      return ITER_FINISHED;
    slot = pvp_object_get_slot(object, iter);
  } while (pv_get_kind(slot->string) == PV_KIND_NULL);
  assert(pv_get_kind(pvp_object_get_slot(object,iter)->string)
         == PV_KIND_STRING);
  return iter;
}

pv pv_object_iter_key(pv object, int iter) {
  pv s = pvp_object_get_slot(object, iter)->string;
  assert(PVP_HAS_KIND(s, PV_KIND_STRING));
  return pv_ref(s);
}

pv pv_object_iter_value(pv object, int iter) {
  return pv_ref(pvp_object_get_slot(object, iter)->value);
}
