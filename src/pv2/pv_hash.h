#ifndef PV_HASH_H
#define PV_HASH_H

#include "pv.h"

#include <stdint.h>

typedef uint32_t (*pv_hash_func)(pv);

void pv_register_hash(pv_kind, pv_hash_func);
uint32_t pv_hash(pv);

uint32_t pvp_hash_data(unsigned char *data, uint32_t len); // uhhhhh where should this go? (don't use this directly in code thanks)

#endif