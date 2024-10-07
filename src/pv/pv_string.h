#ifndef PV_STRING_H
#define PV_STRING_H

#include "pv.h"

#include <stdint.h>

extern pv_kind string_kind;

void pv_string_install();

pv pv_string(const char*);
uint32_t pv_string_length(pv);
pv pv_string_concat(pv, pv);

#endif