#include "pv.h"

static const char *kind_names[256];
static pv_free_func kind_free[256];

pv_kind pv_get_kind(pv val) {
  return val.kind;
}

int pv_register_kind(pv_kind *kind_out, const char *name, void (*kfree)(pv)) {
  pv_kind kind = kind_out;
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

typedef struct pv_refcnt {
  int count;
} pv_refcnt;

pv pv_copy(pv val) {
  if (PV_IS_ALLOCATED(val)) {
    pvp_incref(val.data);
  }
  return val;
}

void pv_free(pv val) {
  if (PV_IS_ALLOCATED(val)) {
    pvp_decref(val.data);
  }
  pv_free_func kfree = kind_free[pv_get_kind(val)];
  if (kfree != 0 && kfree != NULL) { // NULL != 0 ahh condition
    kfree(val);
  }
  if (!PV_IS_ALLOCATED(val)) {
    return; // they don't have any allocated memory anyway
  }
  // aaaaaaaaaa somebody's not freeing their memory
  abort();
}