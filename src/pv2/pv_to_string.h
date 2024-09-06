#ifndef PV_TO_STRING_H
#define PV_TO_STRING_H

#include "pv.h"

void pv_register_to_string(pv_kind kind, char *(*f)(pv));
char *pv_to_string(pv val);
#endif