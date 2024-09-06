#ifndef PV_TO_STRING_H
#define PV_TO_STRING_H

#include "pv.h"

typedef char *(*pv_to_string_func)(pv);

void pv_register_to_string(pv_kind, pv_to_string_func);
char *pv_to_string(pv);

#endif