#include "pv.h"

typedef struct pv_refcnt {
  int count;
} pv_refcnt;

int pvp_decref(pv_refcnt* c);
void pvp_incref(pv_refcnt* c);