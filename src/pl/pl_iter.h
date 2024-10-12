#ifndef PL_ITER_H
#define PL_ITER_H

#include "pv.h"

extern pv_kind iter_kind;

void pl_iter_install();

pv pl_iter(pv); // keys for object, values for array (like python because)
pv pl_iter_keys(pv);
pv pl_iter_values(pv);
pv pl_iter_entries(pv); // [key, value]...

pv pl_iter_value(pv); // the current value of the iterator
pv pl_iter_next(pv); // step the iterator

#endif