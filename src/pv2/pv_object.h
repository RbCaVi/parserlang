#ifndef PV_OBJECT_H
#define PV_OBJECT_H

#include "pv.h"

// copied straight out of jq

extern pv_kind object_kind;

pv pv_object(void);
pv pv_object_get(pv object, pv key);
int pv_object_has(pv object, pv key);
pv pv_object_set(pv object, pv key, pv value);
pv pv_object_delete(pv object, pv key);
int pv_object_length(pv object);
pv pv_object_merge(pv, pv);
pv pv_object_merge_recursive(pv, pv);

int pv_object_iter(pv);
int pv_object_iter_next(pv, int);
int pv_object_iter_valid(pv, int);
pv pv_object_iter_key(pv, int);
pv pv_object_iter_value(pv, int);
#define pv_object_foreach(t, k, v)                                      \
  for (int pv_i__ = pv_object_iter(t), pv_j__ = 1; pv_j__; pv_j__ = 0)  \
    for (pv k, v;                                                       \
         pv_object_iter_valid((t), pv_i__) ?                            \
           (k = pv_object_iter_key(t, pv_i__),                          \
            v = pv_object_iter_value(t, pv_i__),                        \
            1)                                                          \
           : 0;                                                         \
         pv_i__ = pv_object_iter_next(t, pv_i__))                       \

#define pv_object_keys_foreach(t, k)                                 \
  for (int pv_i__ = pv_object_iter(t), pv_j__ = 1; pv_j__; pv_j__ = 0)  \
    for (pv k;                                                          \
         pv_object_iter_valid((t), pv_i__) ?                            \
           (k = pv_object_iter_key(t, pv_i__),                          \
            1)                                                          \
           : 0;                                                         \
         pv_i__ = pv_object_iter_next(t, pv_i__))

#define PV_OBJECT_2(k1,v1) (pv_object_set(pv_object(),(k1),(v1)))
#define PV_OBJECT_4(k1,v1,k2,v2) (pv_object_set(PV_OBJECT_2((k1),(v1)),(k2),(v2)))
#define PV_OBJECT_6(k1,v1,k2,v2,k3,v3) (pv_object_set(PV_OBJECT_4((k1),(v1),(k2),(v2)),(k3),(v3)))
#define PV_OBJECT_8(k1,v1,k2,v2,k3,v3,k4,v4) (pv_object_set(PV_OBJECT_6((k1),(v1),(k2),(v2),(k3),(v3)),(k4),(v4)))
#define PV_OBJECT_10(k1,v1,k2,v2,k3,v3,k4,v4,k5,v5) \
    (pv_object_set(PV_OBJECT_8((k1),(v1),(k2),(v2),(k3),(v3),(k4),(v4)),(k5),(v5)))
#define PV_OBJECT_12(k1,v1,k2,v2,k3,v3,k4,v4,k5,v5,k6,v6) \
    (pv_object_set(PV_OBJECT_10((k1),(v1),(k2),(v2),(k3),(v3),(k4),(v4),(k5),(v5)),(k6),(v6)))
#define PV_OBJECT_14(k1,v1,k2,v2,k3,v3,k4,v4,k5,v5,k6,v6,k7,v7) \
    (pv_object_set(PV_OBJECT_12((k1),(v1),(k2),(v2),(k3),(v3),(k4),(v4),(k5),(v5),(k6),(v6)),(k7),(v7)))
#define PV_OBJECT_16(k1,v1,k2,v2,k3,v3,k4,v4,k5,v5,k6,v6,k7,v7,k8,v8) \
    (pv_object_set(PV_OBJECT_14((k1),(v1),(k2),(v2),(k3),(v3),(k4),(v4),(k5),(v5),(k6),(v6),(k7),(v7)),(k8),(v8)))
#define PV_OBJECT_18(k1,v1,k2,v2,k3,v3,k4,v4,k5,v5,k6,v6,k7,v7,k8,v8,k9,v9) \
    (pv_object_set(PV_OBJECT_16((k1),(v1),(k2),(v2),(k3),(v3),(k4),(v4),(k5),(v5),(k6),(v6),(k7),(v7),(k8),(v8)),(k9),(v9)))
#define PV_OBJECT_IDX(_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,_11,_12,_13,_14,_15,_16,_17,_18,NAME,...) NAME
#define PV_OBJECT(...) \
  PV_OBJECT_IDX(__VA_ARGS__, \
                PV_OBJECT_18, uneven_object, PV_OBJECT_16, uneven_object, \
                PV_OBJECT_14, uneven_object, PV_OBJECT_12, uneven_object, \
                PV_OBJECT_10, uneven_object, PV_OBJECT_8, uneven_object,    \
                PV_OBJECT_6, uneven_object, PV_OBJECT_4, uneven_object,     \
                PV_OBJECT_2, uneven_object, pv_object)(__VA_ARGS__)

#endif