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