#include "pv.h"

pv_kind number_kind;

void pv_number_install();

pv pv_number(double);
double pv_number_value(pv);
int pv_is_integer(pv);