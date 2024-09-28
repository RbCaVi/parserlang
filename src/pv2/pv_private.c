#include "pv_private.h"

#include <stdlib.h>

int pvp_getref(pv_refcnt* c) {
  return c->count;
}

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


void *pv_alloc(size_t size) {
  void *p = malloc(size);
  if (p == NULL) {
    abort();
  }
  return p;
}

void *pv_realloc(void *ptr, size_t size) {
  void *p = realloc(ptr, size);
  if (p == NULL) {
    abort();
  }
  return p;
}