#include <assert.h>
#include <limits.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include "pv_alloc.h"
#include "pv_private.h"

// making this static verbose function here
// until we introduce a less confusing naming scheme
// of pv_* API with regards to the memory management
static double pv_number_get_value_and_consume(pv number) {
  double value = pv_number_value(number);
  pv_unref(number);
  return value;
}

static pv parse_slice(pv j, pv slice, int* pstart, int* pend) {
  // Array slices
  pv start_pv = pv_object_get(pv_ref(slice), pv_string("start"));
  pv end_pv = pv_object_get(slice, pv_string("end"));
  if (pv_get_kind(start_pv) == JV_KIND_NULL) {
    pv_unref(start_pv);
    start_pv = pv_number(0);
  }
  int len;
  if (pv_get_kind(j) == JV_KIND_ARRAY) {
    len = pv_array_length(j);
  } else if (pv_get_kind(j) == JV_KIND_STRING) {
    len = pv_string_length_codepoints(j);
  } else {
    /*
     * XXX This should be dead code because callers shouldn't call this
     * function if `j' is neither an array nor a string.
     */
    pv_unref(j);
    pv_unref(start_pv);
    pv_unref(end_pv);
    return pv_invalid_with_msg(pv_string("Only arrays and strings can be sliced"));
  }
  if (pv_get_kind(end_pv) == JV_KIND_NULL) {
    pv_unref(end_pv);
    end_pv = pv_number(len);
  }
  if (pv_get_kind(start_pv) != JV_KIND_NUMBER ||
      pv_get_kind(end_pv) != JV_KIND_NUMBER) {
    pv_unref(start_pv);
    pv_unref(end_pv);
    return pv_invalid_with_msg(pv_string("Array/string slice indices must be integers"));
  }

  double dstart = pv_number_value(start_pv);
  double dend = pv_number_value(end_pv);
  int start, end;

  pv_unref(start_pv);
  pv_unref(end_pv);
  if (isnan(dstart)) dstart = 0;
  if (dstart < 0)    dstart += len;
  if (dstart < 0)    dstart = 0;
  if (dstart > len)  dstart = len;
  start = dstart > INT_MAX ? INT_MAX : (int)dstart; // Rounds down

  if (isnan(dend))   dend = len;
  if (dend < 0)      dend += len;
  if (dend < 0)      dend  = start;
  end = dend > INT_MAX ? INT_MAX : (int)dend;
  if (end > len)     end = len;
  if (end < len)     end += end < dend ? 1 : 0; // We round start down
                                                // but round end up

  if (end < start) end = start;
  assert(0 <= start && start <= end && end <= len);
  *pstart = start;
  *pend = end;
  return pv_true();
}

pv pv_get(pv t, pv k) {
  pv v;
  if (pv_get_kind(t) == JV_KIND_OBJECT && pv_get_kind(k) == JV_KIND_STRING) {
    v = pv_object_get(t, k);
    if (!pv_is_valid(v)) {
      pv_unref(v);
      v = pv_null();
    }
  } else if (pv_get_kind(t) == JV_KIND_ARRAY && pv_get_kind(k) == JV_KIND_NUMBER) {
    if (pvp_number_is_nan(k)) {
      pv_unref(t);
      v = pv_null();
    } else {
      double didx = pv_number_value(k);
      if (pvp_number_is_nan(k)) {
        v = pv_null();
      } else {
        if (didx < INT_MIN) didx = INT_MIN;
        if (didx > INT_MAX) didx = INT_MAX;
        int idx = (int)didx;
        if (idx < 0)
          idx += pv_array_length(pv_ref(t));
        v = pv_array_get(t, idx);
        if (!pv_is_valid(v)) {
          pv_unref(v);
          v = pv_null();
        }
      }
    }
    pv_unref(k);
  } else if (pv_get_kind(t) == JV_KIND_ARRAY && pv_get_kind(k) == JV_KIND_OBJECT) {
    int start, end;
    pv e = parse_slice(pv_ref(t), k, &start, &end);
    if (pv_get_kind(e) == JV_KIND_TRUE) {
      v = pv_array_slice(t, start, end);
    } else {
      pv_unref(t);
      v = e;
    }
  } else if (pv_get_kind(t) == JV_KIND_STRING && pv_get_kind(k) == JV_KIND_OBJECT) {
    int start, end;
    pv e = parse_slice(pv_ref(t), k, &start, &end);
    if (pv_get_kind(e) == JV_KIND_TRUE) {
      v = pv_string_slice(t, start, end);
    } else {
      pv_unref(t);
      v = e;
    }
  } else if (pv_get_kind(t) == JV_KIND_ARRAY && pv_get_kind(k) == JV_KIND_ARRAY) {
    v = pv_array_indexes(t, k);
  } else if (pv_get_kind(t) == JV_KIND_NULL &&
             (pv_get_kind(k) == JV_KIND_STRING ||
              pv_get_kind(k) == JV_KIND_NUMBER ||
              pv_get_kind(k) == JV_KIND_OBJECT)) {
    pv_unref(t);
    pv_unref(k);
    v = pv_null();
  } else {
    /*
     * If k is a short string it's probably from a jq .foo expression or
     * similar, in which case putting it in the invalid msg may help the
     * user.  The length 30 is arbitrary.
     */
    if (pv_get_kind(k) == JV_KIND_STRING && pv_string_length_bytes(pv_ref(k)) < 30) {
      v = pv_invalid_with_msg(pv_string_fmt("Cannot index %s with string \"%s\"",
                                            pv_kind_name(pv_get_kind(t)),
                                            pv_string_value(k)));
    } else {
      v = pv_invalid_with_msg(pv_string_fmt("Cannot index %s with %s",
                                            pv_kind_name(pv_get_kind(t)),
                                            pv_kind_name(pv_get_kind(k))));
    }
    pv_unref(t);
    pv_unref(k);
  }
  return v;
}

pv pv_set(pv t, pv k, pv v) {
  if (!pv_is_valid(v)) {
    pv_unref(t);
    pv_unref(k);
    return v;
  }
  int isnull = pv_get_kind(t) == JV_KIND_NULL;
  if (pv_get_kind(k) == JV_KIND_STRING &&
      (pv_get_kind(t) == JV_KIND_OBJECT || isnull)) {
    if (isnull) t = pv_object();
    t = pv_object_set(t, k, v);
  } else if (pv_get_kind(k) == JV_KIND_NUMBER &&
             (pv_get_kind(t) == JV_KIND_ARRAY || isnull)) {
    if (pvp_number_is_nan(k)) {
      pv_unref(t);
      pv_unref(k);
      t = pv_invalid_with_msg(pv_string("Cannot set array element at NaN index"));
    } else {
      double didx = pv_number_value(k);
      if (didx < INT_MIN) didx = INT_MIN;
      if (didx > INT_MAX) didx = INT_MAX;
      if (isnull) t = pv_array();
      t = pv_array_set(t, (int)didx, v);
      pv_unref(k);
    }
  } else if (pv_get_kind(k) == JV_KIND_OBJECT &&
             (pv_get_kind(t) == JV_KIND_ARRAY || isnull)) {
    if (isnull) t = pv_array();
    int start, end;
    pv e = parse_slice(pv_ref(t), k, &start, &end);
    if (pv_get_kind(e) == JV_KIND_TRUE) {
      if (pv_get_kind(v) == JV_KIND_ARRAY) {
        int array_len = pv_array_length(pv_ref(t));
        assert(0 <= start && start <= end && end <= array_len);
        int slice_len = end - start;
        int insert_len = pv_array_length(pv_ref(v));
        if (slice_len < insert_len) {
          // array is growing
          int shift = insert_len - slice_len;
          for (int i = array_len - 1; i >= end; i--) {
            t = pv_array_set(t, i + shift, pv_array_get(pv_ref(t), i));
          }
        } else if (slice_len > insert_len) {
          // array is shrinking
          int shift = slice_len - insert_len;
          for (int i = end; i < array_len; i++) {
            t = pv_array_set(t, i - shift, pv_array_get(pv_ref(t), i));
          }
          t = pv_array_slice(t, 0, array_len - shift);
        }
        for (int i=0; i < insert_len; i++) {
          t = pv_array_set(t, start + i, pv_array_get(pv_ref(v), i));
        }
        pv_unref(v);
      } else {
        pv_unref(t);
        pv_unref(v);
        t = pv_invalid_with_msg(pv_string_fmt("A slice of an array can only be assigned another array"));
      }
    } else {
      pv_unref(t);
      pv_unref(v);
      t = e;
    }
  } else if (pv_get_kind(k) == JV_KIND_OBJECT && pv_get_kind(t) == JV_KIND_STRING) {
    pv_unref(t);
    pv_unref(k);
    pv_unref(v);
    /* Well, why not?  We should implement this... */
    t = pv_invalid_with_msg(pv_string_fmt("Cannot update string slices"));
  } else {
    pv err = pv_invalid_with_msg(pv_string_fmt("Cannot update field at %s index of %s",
                                               pv_kind_name(pv_get_kind(k)),
                                               pv_kind_name(pv_get_kind(t))));
    pv_unref(t);
    pv_unref(k);
    pv_unref(v);
    t = err;
  }
  return t;
}

pv pv_has(pv t, pv k) {
  assert(pv_is_valid(t));
  assert(pv_is_valid(k));
  pv ret;
  if (pv_get_kind(t) == JV_KIND_NULL) {
    pv_unref(t);
    pv_unref(k);
    ret = pv_false();
  } else if (pv_get_kind(t) == JV_KIND_OBJECT &&
             pv_get_kind(k) == JV_KIND_STRING) {
    pv elem = pv_object_get(t, k);
    ret = pv_bool(pv_is_valid(elem));
    pv_unref(elem);
  } else if (pv_get_kind(t) == JV_KIND_ARRAY &&
             pv_get_kind(k) == JV_KIND_NUMBER) {
    if (pvp_number_is_nan(k)) {
      pv_unref(t);
      ret = pv_false();
    } else {
      pv elem = pv_array_get(t, (int)pv_number_value(k));
      ret = pv_bool(pv_is_valid(elem));
      pv_unref(elem);
    }
    pv_unref(k);
  } else {
    ret = pv_invalid_with_msg(pv_string_fmt("Cannot check whether %s has a %s key",
                                            pv_kind_name(pv_get_kind(t)),
                                            pv_kind_name(pv_get_kind(k))));
    pv_unref(t);
    pv_unref(k);
  }
  return ret;
}

// assumes keys is a sorted array
static pv pv_dels(pv t, pv keys) {
  assert(pv_get_kind(keys) == JV_KIND_ARRAY);
  assert(pv_is_valid(t));

  if (pv_get_kind(t) == JV_KIND_NULL || pv_array_length(pv_ref(keys)) == 0) {
    // no change
  } else if (pv_get_kind(t) == JV_KIND_ARRAY) {
    // extract slices, they must be handled differently
    pv neg_keys = pv_array();
    pv nonneg_keys = pv_array();
    pv new_array = pv_array();
    pv starts = pv_array(), ends = pv_array();
    pv_array_foreach(keys, i, key) {
      if (pv_get_kind(key) == JV_KIND_NUMBER) {
        if (pv_number_value(key) < 0) {
          neg_keys = pv_array_append(neg_keys, key);
        } else {
          nonneg_keys = pv_array_append(nonneg_keys, key);
        }
      } else if (pv_get_kind(key) == JV_KIND_OBJECT) {
        int start, end;
        pv e = parse_slice(pv_ref(t), key, &start, &end);
        if (pv_get_kind(e) == JV_KIND_TRUE) {
          starts = pv_array_append(starts, pv_number(start));
          ends = pv_array_append(ends, pv_number(end));
        } else {
          pv_unref(new_array);
          pv_unref(key);
          new_array = e;
          goto arr_out;
        }
      } else {
        pv_unref(new_array);
        new_array = pv_invalid_with_msg(pv_string_fmt("Cannot delete %s element of array",
                                                      pv_kind_name(pv_get_kind(key))));
        pv_unref(key);
        goto arr_out;
      }
    }

    int neg_idx = 0;
    int nonneg_idx = 0;
    int len = pv_array_length(pv_ref(t));
    pv_array_foreach(t, i, elem) {
      int del = 0;
      while (neg_idx < pv_array_length(pv_ref(neg_keys))) {
        int delidx = len + (int)pv_number_get_value_and_consume(pv_array_get(pv_ref(neg_keys), neg_idx));
        if (i == delidx) {
          del = 1;
        }
        if (i < delidx) {
          break;
        }
        neg_idx++;
      }
      while (nonneg_idx < pv_array_length(pv_ref(nonneg_keys))) {
        int delidx = (int)pv_number_get_value_and_consume(pv_array_get(pv_ref(nonneg_keys), nonneg_idx));
        if (i == delidx) {
          del = 1;
        }
        if (i < delidx) {
          break;
        }
        nonneg_idx++;
      }
      for (int sidx=0; !del && sidx<pv_array_length(pv_ref(starts)); sidx++) {
        if ((int)pv_number_get_value_and_consume(pv_array_get(pv_ref(starts), sidx)) <= i &&
            i < (int)pv_number_get_value_and_consume(pv_array_get(pv_ref(ends), sidx))) {
          del = 1;
        }
      }
      if (!del)
        new_array = pv_array_append(new_array, elem);
      else
        pv_unref(elem);
    }
  arr_out:
    pv_unref(neg_keys);
    pv_unref(nonneg_keys);
    pv_unref(starts);
    pv_unref(ends);
    pv_unref(t);
    t = new_array;
  } else if (pv_get_kind(t) == JV_KIND_OBJECT) {
    pv_array_foreach(keys, i, k) {
      if (pv_get_kind(k) != JV_KIND_STRING) {
        pv_unref(t);
        t = pv_invalid_with_msg(pv_string_fmt("Cannot delete %s field of object",
                                              pv_kind_name(pv_get_kind(k))));
        pv_unref(k);
        break;
      }
      t = pv_object_delete(t, k);
    }
  } else {
    pv err = pv_invalid_with_msg(pv_string_fmt("Cannot delete fields from %s",
                                               pv_kind_name(pv_get_kind(t))));
    pv_unref(t);
    t = err;
  }
  pv_unref(keys);
  return t;
}

pv pv_setpath(pv root, pv path, pv value) {
  if (pv_get_kind(path) != JV_KIND_ARRAY) {
    pv_unref(value);
    pv_unref(root);
    pv_unref(path);
    return pv_invalid_with_msg(pv_string("Path must be specified as an array"));
  }
  if (!pv_is_valid(root)){
    pv_unref(value);
    pv_unref(path);
    return root;
  }
  if (pv_array_length(pv_ref(path)) == 0) {
    pv_unref(path);
    pv_unref(root);
    return value;
  }
  pv pathcurr = pv_array_get(pv_ref(path), 0);
  pv pathrest = pv_array_slice(path, 1, pv_array_length(pv_ref(path)));

  /*
   * We need to be careful not to make extra copies since that leads to
   * quadratic behavior (e.g., when growing large data structures in a
   * reduction with `setpath/2`, i.e., with `|=`.
   */
  if (pv_get_kind(pathcurr) == JV_KIND_OBJECT) {
    // Assignment to slice -- dunno yet how to avoid the extra copy
    return pv_set(root, pathcurr,
                  pv_setpath(pv_get(pv_ref(root), pv_ref(pathcurr)), pathrest, value));
  }

  pv subroot = pv_get(pv_ref(root), pv_ref(pathcurr));
  if (!pv_is_valid(subroot)) {
    pv_unref(root);
    pv_unref(pathcurr);
    pv_unref(pathrest);
    pv_unref(value);
    return subroot;
  }

  // To avoid the extra copy we drop the reference from `root` by setting that
  // to null first.
  root = pv_set(root, pv_ref(pathcurr), pv_null());
  if (!pv_is_valid(root)) {
    pv_unref(pathcurr);
    pv_unref(pathrest);
    pv_unref(value);
    return root;
  }
  return pv_set(root, pathcurr, pv_setpath(subroot, pathrest, value));
}

pv pv_getpath(pv root, pv path) {
  if (pv_get_kind(path) != JV_KIND_ARRAY) {
    pv_unref(root);
    pv_unref(path);
    return pv_invalid_with_msg(pv_string("Path must be specified as an array"));
  }
  if (!pv_is_valid(root)) {
    pv_unref(path);
    return root;
  }
  if (pv_array_length(pv_ref(path)) == 0) {
    pv_unref(path);
    return root;
  }
  pv pathcurr = pv_array_get(pv_ref(path), 0);
  pv pathrest = pv_array_slice(path, 1, pv_array_length(pv_ref(path)));
  return pv_getpath(pv_get(root, pathcurr), pathrest);
}

// assumes paths is a sorted array of arrays
static pv delpaths_sorted(pv object, pv paths, int start) {
  pv delkeys = pv_array();
  for (int i=0; i<pv_array_length(pv_ref(paths));) {
    int j = i;
    assert(pv_array_length(pv_array_get(pv_ref(paths), i)) > start);
    int delkey = pv_array_length(pv_array_get(pv_ref(paths), i)) == start + 1;
    pv key = pv_array_get(pv_array_get(pv_ref(paths), i), start);
    while (j < pv_array_length(pv_ref(paths)) &&
           pv_equal(pv_ref(key), pv_array_get(pv_array_get(pv_ref(paths), j), start)))
      j++;
    // if i <= entry < j, then entry starts with key
    if (delkey) {
      // deleting this entire key, we don't care about any more specific deletions
      delkeys = pv_array_append(delkeys, key);
    } else {
      // deleting certain sub-parts of this key
      pv subobject = pv_get(pv_ref(object), pv_ref(key));
      if (!pv_is_valid(subobject)) {
        pv_unref(key);
        pv_unref(object);
        object = subobject;
        break;
      } else if (pv_get_kind(subobject) == JV_KIND_NULL) {
        pv_unref(key);
        pv_unref(subobject);
      } else {
        pv newsubobject = delpaths_sorted(subobject, pv_array_slice(pv_ref(paths), i, j), start+1);
        if (!pv_is_valid(newsubobject)) {
          pv_unref(key);
          pv_unref(object);
          object = newsubobject;
          break;
        }
        object = pv_set(object, key, newsubobject);
      }
      if (!pv_is_valid(object)) break;
    }
    i = j;
  }
  pv_unref(paths);
  if (pv_is_valid(object))
    object = pv_dels(object, delkeys);
  else
    pv_unref(delkeys);
  return object;
}

pv pv_delpaths(pv object, pv paths) {
  if (pv_get_kind(paths) != JV_KIND_ARRAY) {
    pv_unref(object);
    pv_unref(paths);
    return pv_invalid_with_msg(pv_string("Paths must be specified as an array"));
  }
  paths = pv_sort(paths, pv_ref(paths));
  pv_array_foreach(paths, i, elem) {
    if (pv_get_kind(elem) != JV_KIND_ARRAY) {
      pv_unref(object);
      pv_unref(paths);
      pv err = pv_invalid_with_msg(pv_string_fmt("Path must be specified as array, not %s",
                                                 pv_kind_name(pv_get_kind(elem))));
      pv_unref(elem);
      return err;
    }
    pv_unref(elem);
  }
  if (pv_array_length(pv_ref(paths)) == 0) {
    // nothing is being deleted
    pv_unref(paths);
    return object;
  }
  if (pv_array_length(pv_array_get(pv_ref(paths), 0)) == 0) {
    // everything is being deleted
    pv_unref(paths);
    pv_unref(object);
    return pv_null();
  }
  return delpaths_sorted(object, paths, 0);
}


static int string_cmp(const void* pa, const void* pb){
  const pv* a = pa;
  const pv* b = pb;
  int lena = pv_string_length_bytes(pv_ref(*a));
  int lenb = pv_string_length_bytes(pv_ref(*b));
  int minlen = lena < lenb ? lena : lenb;
  int r = memcmp(pv_string_value(*a), pv_string_value(*b), minlen);
  if (r == 0) r = lena - lenb;
  return r;
}

pv pv_keys_unsorted(pv x) {
  if (pv_get_kind(x) != JV_KIND_OBJECT)
    return pv_keys(x);
  pv answer = pv_array_sized(pv_object_length(pv_ref(x)));
  pv_object_foreach(x, key, value) {
    answer = pv_array_append(answer, key);
    pv_unref(value);
  }
  pv_unref(x);
  return answer;
}

pv pv_keys(pv x) {
  if (pv_get_kind(x) == JV_KIND_OBJECT) {
    int nkeys = pv_object_length(pv_ref(x));
    pv* keys = pv_mem_calloc(nkeys, sizeof(pv));
    int kidx = 0;
    pv_object_foreach(x, key, value) {
      keys[kidx++] = key;
      pv_unref(value);
    }
    qsort(keys, nkeys, sizeof(pv), string_cmp);
    pv answer = pv_array_sized(nkeys);
    for (int i = 0; i<nkeys; i++) {
      answer = pv_array_append(answer, keys[i]);
    }
    pv_mem_free(keys);
    pv_unref(x);
    return answer;
  } else if (pv_get_kind(x) == JV_KIND_ARRAY) {
    int n = pv_array_length(x);
    pv answer = pv_array();
    for (int i=0; i<n; i++){
      answer = pv_array_set(answer, i, pv_number(i));
    }
    return answer;
  } else {
    assert(0 && "pv_keys passed something neither object nor array");
    return pv_invalid();
  }
}

int pv_cmp(pv a, pv b) {
  if (pv_get_kind(a) != pv_get_kind(b)) {
    int r = (int)pv_get_kind(a) - (int)pv_get_kind(b);
    pv_unref(a);
    pv_unref(b);
    return r;
  }
  int r = 0;
  switch (pv_get_kind(a)) {
  default:
    assert(0 && "invalid kind passed to pv_cmp");
  case JV_KIND_NULL:
  case JV_KIND_FALSE:
  case JV_KIND_TRUE:
    // there's only one of each of these values
    r = 0;
    break;

  case JV_KIND_NUMBER: {
    if (pvp_number_is_nan(a)) {
      r = pv_cmp(pv_null(), pv_ref(b));
    } else if (pvp_number_is_nan(b)) {
      r = pv_cmp(pv_ref(a), pv_null());
    } else {
      r = pvp_number_cmp(a, b);
    }
    break;
  }

  case JV_KIND_STRING: {
    r = string_cmp(&a, &b);
    break;
  }

  case JV_KIND_ARRAY: {
    // Lexical ordering of arrays
    int i = 0;
    while (r == 0) {
      int a_done = i >= pv_array_length(pv_ref(a));
      int b_done = i >= pv_array_length(pv_ref(b));
      if (a_done || b_done) {
        r = b_done - a_done; //suddenly, logic
        break;
      }
      pv xa = pv_array_get(pv_ref(a), i);
      pv xb = pv_array_get(pv_ref(b), i);
      r = pv_cmp(xa, xb);
      i++;
    }
    break;
  }

  case JV_KIND_OBJECT: {
    pv keys_a = pv_keys(pv_ref(a));
    pv keys_b = pv_keys(pv_ref(b));
    r = pv_cmp(pv_ref(keys_a), keys_b);
    if (r == 0) {
      pv_array_foreach(keys_a, i, key) {
        pv xa = pv_object_get(pv_ref(a), pv_ref(key));
        pv xb = pv_object_get(pv_ref(b), key);
        r = pv_cmp(xa, xb);
        if (r) break;
      }
    }
    pv_unref(keys_a);
    break;
  }
  }

  pv_unref(a);
  pv_unref(b);
  return r;
}


struct sort_entry {
  pv object;
  pv key;
  int index;
};

static int sort_cmp(const void* pa, const void* pb) {
  const struct sort_entry* a = pa;
  const struct sort_entry* b = pb;
  int r = pv_cmp(pv_ref(a->key), pv_ref(b->key));
  // comparing by index if r == 0 makes the sort stable
  return r ? r : (a->index - b->index);
}

static struct sort_entry* sort_items(pv objects, pv keys) {
  assert(pv_get_kind(objects) == JV_KIND_ARRAY);
  assert(pv_get_kind(keys) == JV_KIND_ARRAY);
  assert(pv_array_length(pv_ref(objects)) == pv_array_length(pv_ref(keys)));
  int n = pv_array_length(pv_ref(objects));
  struct sort_entry* entries = pv_mem_calloc(n, sizeof(struct sort_entry));
  for (int i=0; i<n; i++) {
    entries[i].object = pv_array_get(pv_ref(objects), i);
    entries[i].key = pv_array_get(pv_ref(keys), i);
    entries[i].index = i;
  }
  pv_unref(objects);
  pv_unref(keys);
  qsort(entries, n, sizeof(struct sort_entry), sort_cmp);
  return entries;
}

pv pv_sort(pv objects, pv keys) {
  assert(pv_get_kind(objects) == JV_KIND_ARRAY);
  assert(pv_get_kind(keys) == JV_KIND_ARRAY);
  assert(pv_array_length(pv_ref(objects)) == pv_array_length(pv_ref(keys)));
  int n = pv_array_length(pv_ref(objects));
  struct sort_entry* entries = sort_items(objects, keys);
  pv ret = pv_array();
  for (int i=0; i<n; i++) {
    pv_unref(entries[i].key);
    ret = pv_array_set(ret, i, entries[i].object);
  }
  pv_mem_free(entries);
  return ret;
}

pv pv_group(pv objects, pv keys) {
  assert(pv_get_kind(objects) == JV_KIND_ARRAY);
  assert(pv_get_kind(keys) == JV_KIND_ARRAY);
  assert(pv_array_length(pv_ref(objects)) == pv_array_length(pv_ref(keys)));
  int n = pv_array_length(pv_ref(objects));
  struct sort_entry* entries = sort_items(objects, keys);
  pv ret = pv_array();
  if (n > 0) {
    pv curr_key = entries[0].key;
    pv group = pv_array_append(pv_array(), entries[0].object);
    for (int i = 1; i < n; i++) {
      if (pv_equal(pv_ref(curr_key), pv_ref(entries[i].key))) {
        pv_unref(entries[i].key);
      } else {
        pv_unref(curr_key);
        curr_key = entries[i].key;
        ret = pv_array_append(ret, group);
        group = pv_array();
      }
      group = pv_array_append(group, entries[i].object);
    }
    pv_unref(curr_key);
    ret = pv_array_append(ret, group);
  }
  pv_mem_free(entries);
  return ret;
}
