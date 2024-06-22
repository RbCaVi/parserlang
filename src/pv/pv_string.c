#include "pv_string.h"

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


void pvp_string_free(pv js) {
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