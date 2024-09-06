#ifndef PV_SINGLETONS_H
#define PV_SINGLETONS_H

#include "pv.h"

extern pv_kind null_kind;
extern pv_kind bool_kind;

void pv_singletons_install();

pv pv_null(void);
pv pv_true(void);
pv pv_false(void);
pv pv_bool(int);

#endif