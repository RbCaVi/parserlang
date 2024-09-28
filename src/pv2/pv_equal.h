#ifndef PV_EQUAL_H
#define PV_EQUAL_H

#include "pv.h"

typedef int (*pv_equal_func)(pv, pv);

void pv_register_equal_self(pv_kind, pv_equal_func);
void pv_register_equal(pv_kind, pv_kind, pv_equal_func);
int pv_equal(pv, pv);

#endif