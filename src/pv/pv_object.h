#ifndef PV_OBJECT_H
#define PV_OBJECT_H

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

#define PV_OBJECT_1(k1) (pv_object_set(pv_object(),(k1),pv_null()))
#define PV_OBJECT_2(k1,v1) (pv_object_set(pv_object(),(k1),(v1)))
#define PV_OBJECT_3(k1,v1,k2) (pv_object_set(PV_OBJECT_2((k1),(v1)),(k2),pv_null()))
#define PV_OBJECT_4(k1,v1,k2,v2) (pv_object_set(PV_OBJECT_2((k1),(v1)),(k2),(v2)))
#define PV_OBJECT_5(k1,v1,k2,v2,k3) (pv_object_set(PV_OBJECT_4((k1),(v1),(k2),(v2)),(k3),pv_null()))
#define PV_OBJECT_6(k1,v1,k2,v2,k3,v3) (pv_object_set(PV_OBJECT_4((k1),(v1),(k2),(v2)),(k3),(v3)))
#define PV_OBJECT_7(k1,v1,k2,v2,k3,v3,k4) (pv_object_set(PV_OBJECT_6((k1),(v1),(k2),(v2),(k3),(v3)),(k4),pv_null()))
#define PV_OBJECT_8(k1,v1,k2,v2,k3,v3,k4,v4) (pv_object_set(PV_OBJECT_6((k1),(v1),(k2),(v2),(k3),(v3)),(k4),(v4)))
#define PV_OBJECT_9(k1,v1,k2,v2,k3,v3,k4,v4,k5) \
    (pv_object_set(PV_OBJECT_8((k1),(v1),(k2),(v2),(k3),(v3),(k4),(v4)),(k5),pv_null()))
#define PV_OBJECT_10(k1,v1,k2,v2,k3,v3,k4,v4,k5,v5) \
    (pv_object_set(PV_OBJECT_8((k1),(v1),(k2),(v2),(k3),(v3),(k4),(v4)),(k5),(v5)))
#define PV_OBJECT_11(k1,v1,k2,v2,k3,v3,k4,v4,k5,v5,k6) \
    (pv_object_set(PV_OBJECT_10((k1),(v1),(k2),(v2),(k3),(v3),(k4),(v4),(k5),(v5)),(k6),pv_null()))
#define PV_OBJECT_12(k1,v1,k2,v2,k3,v3,k4,v4,k5,v5,k6,v6) \
    (pv_object_set(PV_OBJECT_10((k1),(v1),(k2),(v2),(k3),(v3),(k4),(v4),(k5),(v5)),(k6),(v6)))
#define PV_OBJECT_13(k1,v1,k2,v2,k3,v3,k4,v4,k5,v5,k6,v6,k7) \
    (pv_object_set(PV_OBJECT_12((k1),(v1),(k2),(v2),(k3),(v3),(k4),(v4),(k5),(v5),(k6),(v6)),(k7),pv_null()))
#define PV_OBJECT_14(k1,v1,k2,v2,k3,v3,k4,v4,k5,v5,k6,v6,k7,v7) \
    (pv_object_set(PV_OBJECT_12((k1),(v1),(k2),(v2),(k3),(v3),(k4),(v4),(k5),(v5),(k6),(v6)),(k7),(v7)))
#define PV_OBJECT_15(k1,v1,k2,v2,k3,v3,k4,v4,k5,v5,k6,v6,k7,v7,k8) \
    (pv_object_set(PV_OBJECT_14((k1),(v1),(k2),(v2),(k3),(v3),(k4),(v4),(k5),(v5),(k6),(v6),(k7),(v7)),(k8),pv_null()))
#define PV_OBJECT_16(k1,v1,k2,v2,k3,v3,k4,v4,k5,v5,k6,v6,k7,v7,k8,v8) \
    (pv_object_set(PV_OBJECT_14((k1),(v1),(k2),(v2),(k3),(v3),(k4),(v4),(k5),(v5),(k6),(v6),(k7),(v7)),(k8),(v8)))
#define PV_OBJECT_17(k1,v1,k2,v2,k3,v3,k4,v4,k5,v5,k6,v6,k7,v7,k8,v8,k9) \
    (pv_object_set(PV_OBJECT_16((k1),(v1),(k2),(v2),(k3),(v3),(k4),(v4),(k5),(v5),(k6),(v6),(k7),(v7),(k8),(v8)),(k9),pv_null()))
#define PV_OBJECT_18(k1,v1,k2,v2,k3,v3,k4,v4,k5,v5,k6,v6,k7,v7,k8,v8,k9,v9) \
    (pv_object_set(PV_OBJECT_16((k1),(v1),(k2),(v2),(k3),(v3),(k4),(v4),(k5),(v5),(k6),(v6),(k7),(v7),(k8),(v8)),(k9),(v9)))
#define PV_OBJECT_IDX(_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,_11,_12,_13,_14,_15,_16,_17,_18,NAME,...) NAME
#define PV_OBJECT(...) \
  PV_OBJECT_IDX(__VA_ARGS__, \
                PV_OBJECT_18, PV_OBJECT_17, PV_OBJECT_16, PV_OBJECT_15, \
                PV_OBJECT_14, PV_OBJECT_13, PV_OBJECT_12, PV_OBJECT_11, \
                PV_OBJECT_10, PV_OBJECT_9, PV_OBJECT_8, PV_OBJECT_7,    \
                PV_OBJECT_6, PV_OBJECT_5, PV_OBJECT_4, PV_OBJECT_3,     \
                PV_OBJECT_2, PV_OBJECT_1)(__VA_ARGS__)

#endif