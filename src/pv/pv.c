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
#include "pvp.h"
#include "pv_unicode.h"
#include "util.h"

/*
 * Internal refcounting helpers
 */

//static const pv_refcnt PV_REFCNT_INIT = {1};

static void pvp_refcnt_inc(pv_refcnt* c) {
  c->count++;
}

int pvp_refcnt_dec(pv_refcnt* c) {
  c->count--;
  return c->count == 0;
}

int pvp_refcnt_unshared(pv_refcnt* c) {
  assert(c->count > 0);
  return c->count == 1;
}

#define PVP_IS_ALLOCATED(j) (j.kind_flags & PVP_PAYLOAD_ALLOCATED)

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
// commented out because i don't want to deal with this
/*
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
*//*
int pv_identical(pv a, pv b) {
  int r;
  if (a.kind_flags != b.kind_flags
      || a.offset != b.offset
      || a.size != b.size) {
    r = 0;
  } else {
    if (PVP_IS_ALLOCATED(a) /* b has the same flags *//*) {
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
*/
