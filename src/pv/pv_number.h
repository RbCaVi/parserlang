#ifndef PV_NUMBER_H
#define PV_NUMBER_H

#include "pv.h"

extern pv_kind double_kind;
extern pv_kind int_kind;

void pv_number_install();

pv pv_double(double);
double pv_double_value(pv);

pv pv_int(int);
int pv_int_value(pv);

double pv_number_value(pv);
int pv_number_int_value(pv);
int pv_is_integer(pv);

#endif
