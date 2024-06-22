#ifndef PV_ARRAY_H
#define PV_ARRAY_H

pv pv_array(void);
pv pv_array_sized(int);
int pv_array_length(pv);
pv pv_array_get(pv, int);
pv pv_array_set(pv, int, pv);
pv pv_array_append(pv, pv);
pv pv_array_concat(pv, pv);
pv pv_array_slice(pv, int, int);
pv pv_array_indexes(pv, pv);
#define pv_array_foreach(a, i, x) \
  for (int pv_len__ = pv_array_length(pv_ref(a)), i=0, pv_j__ = 1;     \
       pv_j__; pv_j__ = 0)                                              \
    for (pv x;                                                          \
         i < pv_len__ ?                                                 \
           (x = pv_array_get(pv_ref(a), i), 1) : 0;                    \
         i++)

#define PV_ARRAY_1(e) (pv_array_append(pv_array(),e))
#define PV_ARRAY_2(e1,e2) (pv_array_append(PV_ARRAY_1(e1),e2))
#define PV_ARRAY_3(e1,e2,e3) (pv_array_append(PV_ARRAY_2(e1,e2),e3))
#define PV_ARRAY_4(e1,e2,e3,e4) (pv_array_append(PV_ARRAY_3(e1,e2,e3),e4))
#define PV_ARRAY_5(e1,e2,e3,e4,e5) (pv_array_append(PV_ARRAY_4(e1,e2,e3,e4),e5))
#define PV_ARRAY_6(e1,e2,e3,e4,e5,e6) (pv_array_append(PV_ARRAY_5(e1,e2,e3,e4,e5),e6))
#define PV_ARRAY_7(e1,e2,e3,e4,e5,e6,e7) (pv_array_append(PV_ARRAY_6(e1,e2,e3,e4,e5,e6),e7))
#define PV_ARRAY_8(e1,e2,e3,e4,e5,e6,e7,e8) (pv_array_append(PV_ARRAY_7(e1,e2,e3,e4,e5,e6,e7),e8))
#define PV_ARRAY_9(e1,e2,e3,e4,e5,e6,e7,e8,e9) (pv_array_append(PV_ARRAY_8(e1,e2,e3,e4,e5,e6,e7,e8),e9))
#define PV_ARRAY_IDX(_1,_2,_3,_4,_5,_6,_7,_8,_9,NAME,...) NAME
#define PV_ARRAY(...) \
  PV_ARRAY_IDX(__VA_ARGS__, PV_ARRAY_9, PV_ARRAY_8, PV_ARRAY_7, PV_ARRAY_6, PV_ARRAY_5, PV_ARRAY_4, PV_ARRAY_3, PV_ARRAY_2, PV_ARRAY_1, dummy)(__VA_ARGS__)

#endif