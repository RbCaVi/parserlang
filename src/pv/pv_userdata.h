#ifndef PV_USERDATA_H
#define PV_USERDATA_H

#include "pv.h"

extern pv_kind userdata_kind;

void pv_userdata_install();

pv pv_userdata(void*);
void *pv_userdata_ptr(pv);

#endif
