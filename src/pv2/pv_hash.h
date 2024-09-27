#ifndef PV_HASH_H
#define PV_HASH_H

#include "pv.h"

#include <stdint.h>

typedef uint32_t (*pv_hash_func)(pv);

void pv_register_hash(pv_kind, pv_hash_func);
uint32_t pv_hash(pv);

#endif