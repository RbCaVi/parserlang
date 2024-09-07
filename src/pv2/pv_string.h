#ifndef PV_STRING_H
#define PV_STRING_H

#include "pv.h"

extern pv_kind string_kind;

void pv_string_install();

pv pv_string(const char*);
int pv_string_length(pv);
unsigned long pv_string_hash(pv);
pv pv_string_concat(pv, pv);

#endif