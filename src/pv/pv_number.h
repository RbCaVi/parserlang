#ifndef PV_NUMBER_H
#define PV_NUMBER_H

#include "pv.h"

extern pv_kind double_kind;
extern pv_kind int_kind;

void pv_number_install();

pv pv_double(double);
double pv_double_value(pv);
int pv_double_is_integer(pv);

pv pv_int(double);
int pv_int_value(pv);

#endif
