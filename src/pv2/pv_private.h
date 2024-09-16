#ifndef PV_PRIVATE_H
#define PV_PRIVATE_H

#include "pv.h"

#include <stddef.h>

typedef struct pv_refcnt {
  int count;
} pv_refcnt;

static const pv_refcnt PV_REFCNT_INIT = {1};

int pvp_decref(pv_refcnt* c);
void pvp_incref(pv_refcnt* c);
int pvp_refcnt_unshared(pv_refcnt* c);

void *pv_alloc(size_t size);
void *pv_realloc(void *ptr, size_t size);

#endif