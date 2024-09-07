#include "pv_private.h"

int pvp_decref(pv_refcnt* c) {
  c->count--;
  return c->count == 0;
}

void pvp_incref(pv_refcnt* c) {
  c->count++;
}

int pvp_refcnt_unshared(pv_refcnt* c) {
  return c->count == 1;
}