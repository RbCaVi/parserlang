/*
 * Portions Copyright (c) 2016 Kungliga Tekniska Högskolan
 * (Royal Institute of Technology, Stockholm, Sweden).
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */


#include <stdint.h>
#include <stddef.h>
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <limits.h>
#include <math.h>
#include <float.h>

#include "pv_alloc.h"
#include "pv.h"
#include "pv_unicode.h"
#include "util.h"

/*
 * Internal refcounting helpers
 */

typedef struct pv_refcnt {
  int count;
} pv_refcnt;

static const pv_refcnt PV_REFCNT_INIT = {1};

static void pvp_refcnt_inc(pv_refcnt* c) {
  c->count++;
}

static int pvp_refcnt_dec(pv_refcnt* c) {
  c->count--;
  return c->count == 0;
}

static int pvp_refcnt_unshared(pv_refcnt* c) {
  assert(c->count > 0);
  return c->count == 1;
}

#define KIND_MASK   0x1F
#define PFLAGS_MASK 0xE0

typedef enum {
  PVP_PAYLOAD_NONE = 0,
  PVP_PAYLOAD_ALLOCATED = 0x80,
} payload_flags;

#define PVP_MAKE_FLAGS(kind, pflags) ((kind & KIND_MASK) | (pflags & PFLAGS_MASK))

#define PVP_FLAGS(j)  ((j).kind_flags)
#define PVP_KIND(j)   (PVP_FLAGS(j) & KIND_MASK)

#define PVP_HAS_FLAGS(j, flags) (PVP_FLAGS(j) == flags)
#define PVP_HAS_KIND(j, kind)   (PVP_KIND(j) == kind)

#define PVP_IS_ALLOCATED(j) (j.kind_flags & PVP_PAYLOAD_ALLOCATED)

#define PVP_FLAGS_NULL      PVP_MAKE_FLAGS(PV_KIND_NULL, PVP_PAYLOAD_NONE)
#define PVP_FLAGS_INVALID   PVP_MAKE_FLAGS(PV_KIND_INVALID, PVP_PAYLOAD_NONE)
#define PVP_FLAGS_FALSE     PVP_MAKE_FLAGS(PV_KIND_FALSE, PVP_PAYLOAD_NONE)
#define PVP_FLAGS_TRUE      PVP_MAKE_FLAGS(PV_KIND_TRUE, PVP_PAYLOAD_NONE)

pv_kind pv_get_kind(pv x) {
  return PVP_KIND(x);
}

const char* pv_kind_name(pv_kind k) {
  switch (k) {
  case PV_KIND_INVALID: return "<invalid>";
  case PV_KIND_NULL:    return "null";
  case PV_KIND_FALSE:   return "boolean";
  case PV_KIND_TRUE:    return "boolean";
  case PV_KIND_NUMBER:  return "number";
  case PV_KIND_STRING:  return "string";
  case PV_KIND_ARRAY:   return "array";
  case PV_KIND_OBJECT:  return "object";
  }
  assert(0 && "invalid kind");
  return "<unknown>";
}

const pv PV_NULL = {PVP_FLAGS_NULL, 0, 0, 0, {0}};
const pv PV_INVALID = {PVP_FLAGS_INVALID, 0, 0, 0, {0}};
const pv PV_FALSE = {PVP_FLAGS_FALSE, 0, 0, 0, {0}};
const pv PV_TRUE = {PVP_FLAGS_TRUE, 0, 0, 0, {0}};

pv pv_true() {
  return PV_TRUE;
}

pv pv_false() {
  return PV_FALSE;
}

pv pv_null() {
  return PV_NULL;
}

pv pv_bool(int x) {
  return x ? PV_TRUE : PV_FALSE;
}

/*
 * Invalid objects, with optional error messages
 */

#define PVP_FLAGS_INVALID_MSG   PVP_MAKE_FLAGS(PV_KIND_INVALID, PVP_PAYLOAD_ALLOCATED)

typedef struct {
  pv_refcnt refcnt;
  pv errmsg;
} pvp_invalid;

pv pv_invalid_with_msg(pv err) {
  pvp_invalid* i = pv_mem_alloc(sizeof(pvp_invalid));
  i->refcnt = PV_REFCNT_INIT;
  i->errmsg = err;

  pv x = {PVP_FLAGS_INVALID_MSG, 0, 0, 0, {&i->refcnt}};
  return x;
}

pv pv_invalid() {
  return PV_INVALID;
}

pv pv_invalid_get_msg(pv inv) {
  assert(PVP_HAS_KIND(inv, PV_KIND_INVALID));

  pv x;
  if (PVP_HAS_FLAGS(inv, PVP_FLAGS_INVALID_MSG)) {
    x = pv_ref(((pvp_invalid*)inv.u.ptr)->errmsg);
  }
  else {
    x = pv_null();
  }

  pv_unref(inv);
  return x;
}

int pv_invalid_has_msg(pv inv) {
  assert(PVP_HAS_KIND(inv, PV_KIND_INVALID));
  int r = PVP_HAS_FLAGS(inv, PVP_FLAGS_INVALID_MSG);
  pv_unref(inv);
  return r;
}

static void pvp_invalid_free(pv x) {
  assert(PVP_HAS_KIND(x, PV_KIND_INVALID));
  if (PVP_HAS_FLAGS(x, PVP_FLAGS_INVALID_MSG) && pvp_refcnt_dec(x.u.ptr)) {
    pv_unref(((pvp_invalid*)x.u.ptr)->errmsg);
    pv_mem_free(x.u.ptr);
  }
}

/*
 * Numbers
 */

pv pv_number(double x) {
  pv j = {
    PV_KIND_NUMBER,
    0, 0, 0, {.number = x}
  };
  return j;
}

double pv_number_value(pv j) {
  assert(PVP_HAS_KIND(j, PV_KIND_NUMBER));
  return j.u.number;
}

int pv_is_integer(pv j){
  if (!PVP_HAS_KIND(j, PV_KIND_NUMBER)){
    return 0;
  }

  double x = pv_number_value(j);

  double ipart;
  double fpart = modf(x, &ipart);

  return fabs(fpart) < DBL_EPSILON;
}

int pvp_number_is_nan(pv n) {
  assert(PVP_HAS_KIND(n, PV_KIND_NUMBER));
  return n.u.number != n.u.number;
}

int pvp_number_cmp(pv a, pv b) {
  assert(PVP_HAS_KIND(a, PV_KIND_NUMBER));
  assert(PVP_HAS_KIND(b, PV_KIND_NUMBER));

  double da = pv_number_value(a), db = pv_number_value(b);
  if (da < db) {
    return -1;
  } else if (da == db) {
    return 0;
  } else {
    return 1;
  }
}

static int pvp_number_equal(pv a, pv b) {
  return pvp_number_cmp(a, b) == 0;
}

/*
 * Arrays (internal helpers)
 */

#define ARRAY_SIZE_ROUND_UP(n) (((n)*3)/2)
#define PVP_FLAGS_ARRAY   PVP_MAKE_FLAGS(PV_KIND_ARRAY, PVP_PAYLOAD_ALLOCATED)

static int imax(int a, int b) {
  if (a>b) return a;
  else return b;
}

//FIXME signed vs unsigned
typedef struct {
  pv_refcnt refcnt;
  int length, alloc_length;
  pv elements[];
} pvp_array;

static pvp_array* pvp_array_ptr(pv a) {
  assert(PVP_HAS_KIND(a, PV_KIND_ARRAY));
  return (pvp_array*)a.u.ptr;
}

static pvp_array* pvp_array_alloc(unsigned size) {
  pvp_array* a = pv_mem_alloc(sizeof(pvp_array) + sizeof(pv) * size);
  a->refcnt.count = 1;
  a->length = 0;
  a->alloc_length = size;
  return a;
}

static pv pvp_array_new(unsigned size) {
  pv r = {PVP_FLAGS_ARRAY, 0, 0, 0, {&pvp_array_alloc(size)->refcnt}};
  return r;
}

static void pvp_array_free(pv a) {
  assert(PVP_HAS_KIND(a, PV_KIND_ARRAY));
  if (pvp_refcnt_dec(a.u.ptr)) {
    pvp_array* array = pvp_array_ptr(a);
    for (int i=0; i<array->length; i++) {
      pv_unref(array->elements[i]);
    }
    pv_mem_free(array);
  }
}

static int pvp_array_length(pv a) {
  assert(PVP_HAS_KIND(a, PV_KIND_ARRAY));
  return a.size;
}

static int pvp_array_offset(pv a) {
  assert(PVP_HAS_KIND(a, PV_KIND_ARRAY));
  return a.offset;
}

static pv* pvp_array_read(pv a, int i) {
  assert(PVP_HAS_KIND(a, PV_KIND_ARRAY));
  if (i >= 0 && i < pvp_array_length(a)) {
    pvp_array* array = pvp_array_ptr(a);
    assert(i + pvp_array_offset(a) < array->length);
    return &array->elements[i + pvp_array_offset(a)];
  } else {
    return 0;
  }
}

static pv* pvp_array_write(pv* a, int i) {
  assert(i >= 0);
  pvp_array* array = pvp_array_ptr(*a);

  int pos = i + pvp_array_offset(*a);
  if (pos < array->alloc_length && pvp_refcnt_unshared(a->u.ptr)) {
    // use existing array space
    for (int j = array->length; j <= pos; j++) {
      array->elements[j] = PV_NULL;
    }
    array->length = imax(pos + 1, array->length);
    a->size = imax(i + 1, a->size);
    return &array->elements[pos];
  } else {
    // allocate a new array
    int new_length = imax(i + 1, pvp_array_length(*a));
    pvp_array* new_array = pvp_array_alloc(ARRAY_SIZE_ROUND_UP(new_length));
    int j;
    for (j = 0; j < pvp_array_length(*a); j++) {
      new_array->elements[j] =
        pv_ref(array->elements[j + pvp_array_offset(*a)]);
    }
    for (; j < new_length; j++) {
      new_array->elements[j] = PV_NULL;
    }
    new_array->length = new_length;
    pvp_array_free(*a);
    pv new_pv = {PVP_FLAGS_ARRAY, 0, 0, new_length, {&new_array->refcnt}};
    *a = new_pv;
    return &new_array->elements[i];
  }
}

static int pvp_array_equal(pv a, pv b) {
  if (pvp_array_length(a) != pvp_array_length(b))
    return 0;
  if (pvp_array_ptr(a) == pvp_array_ptr(b) &&
      pvp_array_offset(a) == pvp_array_offset(b))
    return 1;
  for (int i=0; i<pvp_array_length(a); i++) {
    if (!pv_equal(pv_ref(*pvp_array_read(a, i)),
                  pv_ref(*pvp_array_read(b, i))))
      return 0;
  }
  return 1;
}

static void pvp_clamp_slice_params(int len, int *pstart, int *pend)
{
  if (*pstart < 0) *pstart = len + *pstart;
  if (*pend < 0) *pend = len + *pend;

  if (*pstart < 0) *pstart = 0;
  if (*pstart > len) *pstart = len;
  if (*pend > len) *pend = len;
  if (*pend < *pstart) *pend = *pstart;
}


static int pvp_array_contains(pv a, pv b) {
  int r = 1;
  pv_array_foreach(b, bi, belem) {
    int ri = 0;
    pv_array_foreach(a, ai, aelem) {
      if (pv_contains(aelem, pv_ref(belem))) {
        ri = 1;
        break;
      }
    }
    pv_unref(belem);
    if (!ri) {
      r = 0;
      break;
    }
  }
  return r;
}


/*
 * Public
 */

static pv pvp_array_slice(pv a, int start, int end) {
  assert(PVP_HAS_KIND(a, PV_KIND_ARRAY));
  int len = pvp_array_length(a);
  pvp_clamp_slice_params(len, &start, &end);
  assert(0 <= start && start <= end && end <= len);

  // FIXME: maybe slice should reallocate if the slice is small enough
  if (start == end) {
    pv_unref(a);
    return pv_array();
  }

  if (a.offset + start >= 1 << (sizeof(a.offset) * CHAR_BIT)) {
    pv r = pv_array_sized(end - start);
    for (int i = start; i < end; i++)
      r = pv_array_append(r, pv_array_get(pv_ref(a), i));
    pv_unref(a);
    return r;
  } else {
    a.offset += start;
    a.size = end - start;
    return a;
  }
}

/*
 * Arrays (public interface)
 */

pv pv_array_sized(int n) {
  return pvp_array_new(n);
}

pv pv_array() {
  return pv_array_sized(16);
}

int pv_array_length(pv j) {
  assert(PVP_HAS_KIND(j, PV_KIND_ARRAY));
  int len = pvp_array_length(j);
  pv_unref(j);
  return len;
}

pv pv_array_get(pv j, int idx) {
  assert(PVP_HAS_KIND(j, PV_KIND_ARRAY));
  pv* slot = pvp_array_read(j, idx);
  pv val;
  if (slot) {
    val = pv_ref(*slot);
  } else {
    val = pv_invalid();
  }
  pv_unref(j);
  return val;
}

pv pv_array_set(pv j, int idx, pv val) {
  assert(PVP_HAS_KIND(j, PV_KIND_ARRAY));

  if (idx < 0)
    idx = pvp_array_length(j) + idx;
  if (idx < 0) {
    pv_unref(j);
    pv_unref(val);
    return pv_invalid_with_msg(pv_string("Out of bounds negative array index"));
  }
  // copy/free of val,j coalesced
  pv* slot = pvp_array_write(&j, idx);
  pv_unref(*slot);
  *slot = val;
  return j;
}

pv pv_array_append(pv j, pv val) {
  // copy/free of val,j coalesced
  return pv_array_set(j, pv_array_length(pv_ref(j)), val);
}

pv pv_array_concat(pv a, pv b) {
  assert(PVP_HAS_KIND(a, PV_KIND_ARRAY));
  assert(PVP_HAS_KIND(b, PV_KIND_ARRAY));

  // FIXME: could be faster
  pv_array_foreach(b, i, elem) {
    a = pv_array_append(a, elem);
  }
  pv_unref(b);
  return a;
}

pv pv_array_slice(pv a, int start, int end) {
  assert(PVP_HAS_KIND(a, PV_KIND_ARRAY));
  // copy/free of a coalesced
  return pvp_array_slice(a, start, end);
}

pv pv_array_indexes(pv a, pv b) {
  pv res = pv_array();
  int idx = -1;
  pv_array_foreach(a, ai, aelem) {
    pv_unref(aelem);
    pv_array_foreach(b, bi, belem) {
      if (!pv_equal(pv_array_get(pv_ref(a), ai + bi), pv_ref(belem)))
        idx = -1;
      else if (bi == 0 && idx == -1)
        idx = ai;
      pv_unref(belem);
    }
    if (idx > -1)
      res = pv_array_append(res, pv_number(idx));
    idx = -1;
  }
  pv_unref(a);
  pv_unref(b);
  return res;
}

/*
 * Strings (internal helpers)
 */

#define PVP_FLAGS_STRING  PVP_MAKE_FLAGS(PV_KIND_STRING, PVP_PAYLOAD_ALLOCATED)

typedef struct {
  pv_refcnt refcnt;
  uint32_t hash;
  // high 31 bits are length, low bit is a flag
  // indicating whether hash has been computed.
  uint32_t length_hashed;
  uint32_t alloc_length;
  char data[];
} pvp_string;

static pvp_string* pvp_string_ptr(pv a) {
  assert(PVP_HAS_KIND(a, PV_KIND_STRING));
  return (pvp_string*)a.u.ptr;
}

static pvp_string* pvp_string_alloc(uint32_t size) {
  pvp_string* s = pv_mem_alloc(sizeof(pvp_string) + size + 1);
  s->refcnt.count = 1;
  s->alloc_length = size;
  return s;
}

/* Copy a UTF8 string, replacing all badly encoded points with U+FFFD */
static pv pvp_string_copy_replace_bad(const char* data, uint32_t length) {
  const char* end = data + length;
  const char* i = data;

  uint32_t maxlength = length * 3 + 1; // worst case: all bad bytes, each becomes a 3-byte U+FFFD
  pvp_string* s = pvp_string_alloc(maxlength);
  char* out = s->data;
  int c = 0;

  while ((i = pvp_utf8_next(i, end, &c))) {
    if (c == -1) {
      c = 0xFFFD; // U+FFFD REPLACEMENT CHARACTER
    }
    out += pvp_utf8_encode(c, out);
    assert(out < s->data + maxlength);
  }
  length = out - s->data;
  s->data[length] = 0;
  s->length_hashed = length << 1;
  pv r = {PVP_FLAGS_STRING, 0, 0, 0, {&s->refcnt}};
  return r;
}

/* Assumes valid UTF8 */
static pv pvp_string_new(const char* data, uint32_t length) {
  pvp_string* s = pvp_string_alloc(length);
  s->length_hashed = length << 1;
  if (data != NULL)
    memcpy(s->data, data, length);
  s->data[length] = 0;
  pv r = {PVP_FLAGS_STRING, 0, 0, 0, {&s->refcnt}};
  return r;
}

static pv pvp_string_empty_new(uint32_t length) {
  pvp_string* s = pvp_string_alloc(length);
  s->length_hashed = 0;
  memset(s->data, 0, length);
  pv r = {PVP_FLAGS_STRING, 0, 0, 0, {&s->refcnt}};
  return r;
}


static void pvp_string_free(pv js) {
  pvp_string* s = pvp_string_ptr(js);
  if (pvp_refcnt_dec(&s->refcnt)) {
    pv_mem_free(s);
  }
}

static uint32_t pvp_string_length(pvp_string* s) {
  return s->length_hashed >> 1;
}

static uint32_t pvp_string_remaining_space(pvp_string* s) {
  assert(s->alloc_length >= pvp_string_length(s));
  uint32_t r = s->alloc_length - pvp_string_length(s);
  return r;
}

static pv pvp_string_append(pv string, const char* data, uint32_t len) {
  pvp_string* s = pvp_string_ptr(string);
  uint32_t currlen = pvp_string_length(s);

  if (pvp_refcnt_unshared(string.u.ptr) &&
      pvp_string_remaining_space(s) >= len) {
    // the next string fits at the end of a
    memcpy(s->data + currlen, data, len);
    s->data[currlen + len] = 0;
    s->length_hashed = (currlen + len) << 1;
    return string;
  } else {
    // allocate a bigger buffer and copy
    uint32_t allocsz = (currlen + len) * 2;
    if (allocsz < 32) allocsz = 32;
    pvp_string* news = pvp_string_alloc(allocsz);
    news->length_hashed = (currlen + len) << 1;
    memcpy(news->data, s->data, currlen);
    memcpy(news->data + currlen, data, len);
    news->data[currlen + len] = 0;
    pvp_string_free(string);
    pv r = {PVP_FLAGS_STRING, 0, 0, 0, {&news->refcnt}};
    return r;
  }
}

static const uint32_t HASH_SEED = 0x432A9843;

static uint32_t rotl32 (uint32_t x, int8_t r){
  return (x << r) | (x >> (32 - r));
}

static uint32_t pvp_string_hash(pv jstr) {
  pvp_string* str = pvp_string_ptr(jstr);
  if (str->length_hashed & 1)
    return str->hash;

  /* The following is based on MurmurHash3.
     MurmurHash3 was written by Austin Appleby, and is placed
     in the public domain. */

  const uint8_t* data = (const uint8_t*)str->data;
  int len = (int)pvp_string_length(str);
  const int nblocks = len / 4;

  uint32_t h1 = HASH_SEED;

  const uint32_t c1 = 0xcc9e2d51;
  const uint32_t c2 = 0x1b873593;
  const uint32_t* blocks = (const uint32_t *)(data + nblocks*4);

  for(int i = -nblocks; i; i++) {
    uint32_t k1 = blocks[i]; //FIXME: endianness/alignment

    k1 *= c1;
    k1 = rotl32(k1,15);
    k1 *= c2;

    h1 ^= k1;
    h1 = rotl32(h1,13);
    h1 = h1*5+0xe6546b64;
  }

  const uint8_t* tail = (const uint8_t*)(data + nblocks*4);

  uint32_t k1 = 0;

  switch(len & 3) {
  case 3: k1 ^= tail[2] << 16;
          JQ_FALLTHROUGH;
  case 2: k1 ^= tail[1] << 8;
          JQ_FALLTHROUGH;
  case 1: k1 ^= tail[0];
          k1 *= c1; k1 = rotl32(k1,15); k1 *= c2; h1 ^= k1;
  }

  h1 ^= len;

  h1 ^= h1 >> 16;
  h1 *= 0x85ebca6b;
  h1 ^= h1 >> 13;
  h1 *= 0xc2b2ae35;
  h1 ^= h1 >> 16;

  str->length_hashed |= 1;
  str->hash = h1;

  return h1;
}


static int pvp_string_equal(pv a, pv b) {
  assert(PVP_HAS_KIND(a, PV_KIND_STRING));
  assert(PVP_HAS_KIND(b, PV_KIND_STRING));
  pvp_string* stra = pvp_string_ptr(a);
  pvp_string* strb = pvp_string_ptr(b);
  if (pvp_string_length(stra) != pvp_string_length(strb)) return 0;
  return memcmp(stra->data, strb->data, pvp_string_length(stra)) == 0;
}

/*
 * Strings (public API)
 */

pv pv_string_sized(const char* str, int len) {
  return
    pvp_utf8_is_valid(str, str+len) ?
    pvp_string_new(str, len) :
    pvp_string_copy_replace_bad(str, len);
}

pv pv_string_empty(int len) {
  return pvp_string_empty_new(len);
}

pv pv_string(const char* str) {
  return pv_string_sized(str, strlen(str));
}

int pv_string_length_bytes(pv j) {
  assert(PVP_HAS_KIND(j, PV_KIND_STRING));
  int r = pvp_string_length(pvp_string_ptr(j));
  pv_unref(j);
  return r;
}

int pv_string_length_codepoints(pv j) {
  assert(PVP_HAS_KIND(j, PV_KIND_STRING));
  const char* i = pv_string_value(j);
  const char* end = i + pv_string_length_bytes(pv_ref(j));
  int c = 0, len = 0;
  while ((i = pvp_utf8_next(i, end, &c))) len++;
  pv_unref(j);
  return len;
}


pv pv_string_indexes(pv j, pv k) {
  assert(PVP_HAS_KIND(j, PV_KIND_STRING));
  assert(PVP_HAS_KIND(k, PV_KIND_STRING));
  const char *jstr = pv_string_value(j);
  const char *idxstr = pv_string_value(k);
  const char *p;
  int jlen = pv_string_length_bytes(pv_ref(j));
  int idxlen = pv_string_length_bytes(pv_ref(k));
  pv a = pv_array();

  if (idxlen != 0) {
    p = jstr;
    while ((p = _jq_memmem(p, (jstr + jlen) - p, idxstr, idxlen)) != NULL) {
      a = pv_array_append(a, pv_number(p - jstr));
      p++;
    }
  }
  pv_unref(j);
  pv_unref(k);
  return a;
}

pv pv_string_split(pv j, pv sep) {
  assert(PVP_HAS_KIND(j, PV_KIND_STRING));
  assert(PVP_HAS_KIND(sep, PV_KIND_STRING));
  const char *jstr = pv_string_value(j);
  const char *jend = jstr + pv_string_length_bytes(pv_ref(j));
  const char *sepstr = pv_string_value(sep);
  const char *p, *s;
  int seplen = pv_string_length_bytes(pv_ref(sep));
  pv a = pv_array();

  assert(pv_get_refcnt(a) == 1);

  if (seplen == 0) {
    int c;
    while ((jstr = pvp_utf8_next(jstr, jend, &c)))
      a = pv_array_append(a, pv_string_append_codepoint(pv_string(""), c));
  } else {
    for (p = jstr; p < jend; p = s + seplen) {
      s = _jq_memmem(p, jend - p, sepstr, seplen);
      if (s == NULL)
        s = jend;
      a = pv_array_append(a, pv_string_sized(p, s - p));
      // Add an empty string to denote that j ends on a sep
      if (s + seplen == jend && seplen != 0)
        a = pv_array_append(a, pv_string(""));
    }
  }
  pv_unref(j);
  pv_unref(sep);
  return a;
}

pv pv_string_explode(pv j) {
  assert(PVP_HAS_KIND(j, PV_KIND_STRING));
  const char* i = pv_string_value(j);
  int len = pv_string_length_bytes(pv_ref(j));
  const char* end = i + len;
  pv a = pv_array_sized(len);
  int c;
  while ((i = pvp_utf8_next(i, end, &c)))
    a = pv_array_append(a, pv_number(c));
  pv_unref(j);
  return a;
}

pv pv_string_implode(pv j) {
  assert(PVP_HAS_KIND(j, PV_KIND_ARRAY));
  int len = pv_array_length(pv_ref(j));
  pv s = pv_string_empty(len);
  int i;

  assert(len >= 0);

  for (i = 0; i < len; i++) {
    pv n = pv_array_get(pv_ref(j), i);
    assert(PVP_HAS_KIND(n, PV_KIND_NUMBER));
    int nv = pv_number_value(n);
    pv_unref(n);
    // outside codepoint range or in utf16 surrogate pair range
    if (nv < 0 || nv > 0x10FFFF || (nv >= 0xD800 && nv <= 0xDFFF))
      nv = 0xFFFD; // U+FFFD REPLACEMENT CHARACTER
    s = pv_string_append_codepoint(s, nv);
  }

  pv_unref(j);
  return s;
}

unsigned long pv_string_hash(pv j) {
  assert(PVP_HAS_KIND(j, PV_KIND_STRING));
  uint32_t hash = pvp_string_hash(j);
  pv_unref(j);
  return hash;
}

const char* pv_string_value(pv j) {
  assert(PVP_HAS_KIND(j, PV_KIND_STRING));
  return pvp_string_ptr(j)->data;
}

pv pv_string_slice(pv j, int start, int end) {
  assert(PVP_HAS_KIND(j, PV_KIND_STRING));
  const char *s = pv_string_value(j);
  int len = pv_string_length_bytes(pv_ref(j));
  int i;
  const char *p, *e;
  int c;
  pv res;

  pvp_clamp_slice_params(len, &start, &end);
  assert(0 <= start && start <= end && end <= len);

  /* Look for byte offset corresponding to start codepoints */
  for (p = s, i = 0; i < start; i++) {
    p = pvp_utf8_next(p, s + len, &c);
    if (p == NULL) {
      pv_unref(j);
      return pv_string_empty(16);
    }
    if (c == -1) {
      pv_unref(j);
      return pv_invalid_with_msg(pv_string("Invalid UTF-8 string"));
    }
  }
  /* Look for byte offset corresponding to end codepoints */
  for (e = p; e != NULL && i < end; i++) {
    e = pvp_utf8_next(e, s + len, &c);
    if (e == NULL) {
      e = s + len;
      break;
    }
    if (c == -1) {
      pv_unref(j);
      return pv_invalid_with_msg(pv_string("Invalid UTF-8 string"));
    }
  }

  /*
   * NOTE: Ideally we should do here what pvp_array_slice() does instead
   * of allocating a new string as we do!  However, we assume NUL-
   * terminated strings all over, and in the pv API, so for now we waste
   * memory like a drunken navy programmer.  There's probably nothing we
   * can do about it.
   */
  res = pv_string_sized(p, e - p);
  pv_unref(j);
  return res;
}

pv pv_string_concat(pv a, pv b) {
  a = pvp_string_append(a, pv_string_value(b),
                        pvp_string_length(pvp_string_ptr(b)));
  pv_unref(b);
  return a;
}

pv pv_string_append_buf(pv a, const char* buf, int len) {
  if (pvp_utf8_is_valid(buf, buf+len)) {
    a = pvp_string_append(a, buf, len);
  } else {
    pv b = pvp_string_copy_replace_bad(buf, len);
    a = pv_string_concat(a, b);
  }
  return a;
}

pv pv_string_append_codepoint(pv a, uint32_t c) {
  char buf[5];
  int len = pvp_utf8_encode(c, buf);
  a = pvp_string_append(a, buf, len);
  return a;
}

pv pv_string_append_str(pv a, const char* str) {
  return pv_string_append_buf(a, str, strlen(str));
}

pv pv_string_vfmt(const char* fmt, va_list ap) {
  int size = 1024;
  while (1) {
    char* buf = pv_mem_alloc(size);
    va_list ap2;
    va_copy(ap2, ap);
    int n = vsnprintf(buf, size, fmt, ap2);
    va_end(ap2);
    /*
     * NOTE: here we support old vsnprintf()s that return -1 because the
     * buffer is too small.
     */
    if (n >= 0 && n < size) {
      pv ret = pv_string_sized(buf, n);
      pv_mem_free(buf);
      return ret;
    } else {
      pv_mem_free(buf);
      size = (n > 0) ? /* standard */ (n * 2) : /* not standard */ (size * 2);
    }
  }
}

pv pv_string_fmt(const char* fmt, ...) {
  va_list args;
  va_start(args, fmt);
  pv res = pv_string_vfmt(fmt, args);
  va_end(args);
  return res;
}

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

static void pvp_object_free(pv o) {
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

/*
 * Memory management
 */
pv pv_ref(pv j) {
  if (PVP_IS_ALLOCATED(j)) {
    pvp_refcnt_inc(j.u.ptr);
  }
  return j;
}

void pv_unref(pv j) {
  switch(PVP_KIND(j)) {
    case PV_KIND_ARRAY:
      pvp_array_free(j);
      break;
    case PV_KIND_STRING:
      pvp_string_free(j);
      break;
    case PV_KIND_OBJECT:
      pvp_object_free(j);
      break;
    case PV_KIND_INVALID:
      pvp_invalid_free(j);
      break;
  }
}

int pv_get_refcnt(pv j) {
  if (PVP_IS_ALLOCATED(j)) {
    return j.u.ptr->count;
  } else {
    return 1;
  }
}

/*
 * Higher-level operations
 */

int pv_equal(pv a, pv b) {
  int r;
  if (pv_get_kind(a) != pv_get_kind(b)) {
    r = 0;
  } else if (PVP_IS_ALLOCATED(a) &&
             PVP_IS_ALLOCATED(b) &&
             a.kind_flags == b.kind_flags &&
             a.size == b.size &&
             a.u.ptr == b.u.ptr) {
    r = 1;
  } else {
    switch (pv_get_kind(a)) {
    case PV_KIND_NUMBER:
      r = pvp_number_equal(a, b);
      break;
    case PV_KIND_ARRAY:
      r = pvp_array_equal(a, b);
      break;
    case PV_KIND_STRING:
      r = pvp_string_equal(a, b);
      break;
    case PV_KIND_OBJECT:
      r = pvp_object_equal(a, b);
      break;
    default:
      r = 1;
      break;
    }
  }
  pv_unref(a);
  pv_unref(b);
  return r;
}

int pv_identical(pv a, pv b) {
  int r;
  if (a.kind_flags != b.kind_flags
      || a.offset != b.offset
      || a.size != b.size) {
    r = 0;
  } else {
    if (PVP_IS_ALLOCATED(a) /* b has the same flags */) {
      r = a.u.ptr == b.u.ptr;
    } else {
      r = memcmp(&a.u.ptr, &b.u.ptr, sizeof(a.u)) == 0;
    }
  }
  pv_unref(a);
  pv_unref(b);
  return r;
}

int pv_contains(pv a, pv b) {
  int r = 1;
  if (pv_get_kind(a) != pv_get_kind(b)) {
    r = 0;
  } else if (PVP_HAS_KIND(a, PV_KIND_OBJECT)) {
    r = pvp_object_contains(a, b);
  } else if (PVP_HAS_KIND(a, PV_KIND_ARRAY)) {
    r = pvp_array_contains(a, b);
  } else if (PVP_HAS_KIND(a, PV_KIND_STRING)) {
    int b_len = pv_string_length_bytes(pv_ref(b));
    if (b_len != 0) {
      r = _jq_memmem(pv_string_value(a), pv_string_length_bytes(pv_ref(a)),
                     pv_string_value(b), b_len) != 0;
    } else {
      r = 1;
    }
  } else {
    r = pv_equal(pv_ref(a), pv_ref(b));
  }
  pv_unref(a);
  pv_unref(b);
  return r;
}
