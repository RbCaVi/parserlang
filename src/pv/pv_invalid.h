#ifndef PV_INVALID_H
#define PV_INVALID_H

#include "pv.h"

extern const pv PV_INVALID;

pv pv_invalid(void);
pv pv_invalid_with_msg(pv);
pv pv_invalid_get_msg(pv);
int pv_invalid_has_msg(pv);

#endif