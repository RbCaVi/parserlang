#ifndef PV_CONSTANTS_H
#define PV_CONSTANTS_H

#include "pv.h"

extern const pv PV_NULL;
extern const pv PV_FALSE;
extern const pv PV_TRUE;

pv pv_null(void);
pv pv_true(void);
pv pv_false(void);
pv pv_bool(int);

#endif