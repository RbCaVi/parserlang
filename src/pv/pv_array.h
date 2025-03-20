#ifndef PV_ARRAY_H
#define PV_ARRAY_H

// copied straight out of jq

#include "pv.h"

#include <stdint.h>

extern pv_kind array_kind;

void pv_array_install();

pv pv_array(void);
pv pv_array_sized(uint32_t);
uint32_t pv_array_length(pv);
pv pv_array_get(pv, int);
pv pv_array_set(pv, int, pv);
pv pv_array_append(pv, pv);
pv pv_array_concat(pv, pv);
//pv pv_array_elements(int, ...); // count + elements (be careful?)

pv *pv_array_data(pv); // get the data from an array (still decrefs it so you have to copy it first)

#define pv_array_foreach(a, i, x) \
  for (uint32_t pv_len__ = pv_array_length(pv_copy(a)), i = 0, pv_j__ = 1; \
       pv_j__; pv_j__ = 0)                                           \
    for (pv x;                                                       \
         i < pv_len__ ?                                              \
           (x = pv_array_get(pv_copy(a), (int)i), 1) : 0;                  \
         i++)

#define PV_ARRAY_1(s,e) (pv_array_append(pv_array_sized(s+1),e))
#define PV_ARRAY_2(s,e1,e2) (pv_array_append(PV_ARRAY_1(s+1,e1),e2))
#define PV_ARRAY_3(s,e1,e2,e3) (pv_array_append(PV_ARRAY_2(s+1,e1,e2),e3))
#define PV_ARRAY_4(s,e1,e2,e3,e4) (pv_array_append(PV_ARRAY_3(s+1,e1,e2,e3),e4))
#define PV_ARRAY_5(s,e1,e2,e3,e4,e5) (pv_array_append(PV_ARRAY_4(s+1,e1,e2,e3,e4),e5))
#define PV_ARRAY_6(s,e1,e2,e3,e4,e5,e6) (pv_array_append(PV_ARRAY_5(s+1,e1,e2,e3,e4,e5),e6))
#define PV_ARRAY_7(s,e1,e2,e3,e4,e5,e6,e7) (pv_array_append(PV_ARRAY_6(s+1,e1,e2,e3,e4,e5,e6),e7))
#define PV_ARRAY_8(s,e1,e2,e3,e4,e5,e6,e7,e8) (pv_array_append(PV_ARRAY_7(s+1,e1,e2,e3,e4,e5,e6,e7),e8))
#define PV_ARRAY_9(s,e1,e2,e3,e4,e5,e6,e7,e8,e9) (pv_array_append(PV_ARRAY_8(s+1,e1,e2,e3,e4,e5,e6,e7,e8),e9))
#define PV_ARRAY_IDX(_1,_2,_3,_4,_5,_6,_7,_8,_9,NAME,...) NAME
#define PV_ARRAY(...) \
  PV_ARRAY_IDX(__VA_ARGS__, PV_ARRAY_9, PV_ARRAY_8, PV_ARRAY_7, PV_ARRAY_6, PV_ARRAY_5, PV_ARRAY_4, PV_ARRAY_3, PV_ARRAY_2, PV_ARRAY_1, dummy)(0,__VA_ARGS__)

#endif