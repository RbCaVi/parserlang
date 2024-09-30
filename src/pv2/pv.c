#include "pv.h"
#include "pv_private.h"

#include <stddef.h>
#include <stdlib.h>

static const char *kind_names[256] = {"INVALID"};
static pv_free_func kind_free[256];

pv_kind pv_get_kind(pv val) {
  pv_kind out = val.kind;
  return out;
}

int pv_register_kind(pv_kind *kind_out, const char *name, pv_free_func kfree) {
  pv_kind kind = *kind_out;
  while (kind_names[kind] != 0) {
    if (kind == 255) {
      kind = 0;
    } else {
      kind++;
    }
  }
  kind_names[kind] = name;
  kind_free[kind] = kfree;
  *kind_out = kind;
  return 0;
}

const char *pv_kind_name(pv_kind kind) {
  const char *name = kind_names[kind];
  if (name != 0) { // "NULL doesn't have to be 0" ahh code
    return name;
  }
  return NULL;
}

pv pv_copy(pv val) {
  if (PV_IS_ALLOCATED(val)) {
    pvp_incref(val.data);
  }
  return val;
}

static int freeing = 0;

void pv_free(pv val) {
  if (PV_IS_ALLOCATED(val)) {
    if (!pvp_decref(val.data)) {
      // not actually freed
      return;
    }
  }
  pv_free_func kfree = kind_free[pv_get_kind(val)];
  if (kfree != 0 && kfree != NULL) { // NULL != 0 ahh condition
    kfree(val);
    return;
  }
  if (!PV_IS_ALLOCATED(val)) {
    return; // they don't have any allocated memory anyway
  }
  // aaaaaaaaaa somebody's not freeing their memory
  abort();
}

int pv_get_refcount(pv val) {
  if (!PV_IS_ALLOCATED(val)) {
    return -1; // no refcount
  }
  return pvp_getref(val.data);
}

pv pv_invalid(void) {
  pv val = {0, 0, NULL}; // invalid kind is always 0
  return val;
}