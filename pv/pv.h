#ifndef PV_H
#define PV_H

#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

#if (defined(__GNUC__) && __GNUC__ >= 7) || \
    (defined(__clang__) && __clang_major__ >= 10)
# define JQ_FALLTHROUGH __attribute__((fallthrough))
#else
# define JQ_FALLTHROUGH do {} while (0) /* fallthrough */
#endif

typedef enum {
  PV_KIND_INVALID,
  PV_KIND_NULL,
  PV_KIND_FALSE,
  PV_KIND_TRUE,
  PV_KIND_NUMBER,
  PV_KIND_STRING,
  PV_KIND_ARRAY,
  PV_KIND_OBJECT
} pv_kind;

struct pv_refcnt;

/* All of the fields of this struct are private.
   Really. Do not play with them. */
typedef struct {
  unsigned char kind_flags;
  unsigned char pad_;
  unsigned short offset;  /* array offsets */
  int size;
  union {
    struct pv_refcnt* ptr;
    double number;
  } u;
} pv;

/*
 * All pv_* functions consume (decref) input and produce (incref) output
 * Except pv_ref
 */

pv_kind pv_get_kind(pv);
const char* pv_kind_name(pv_kind);
static int pv_is_valid(pv x) { return pv_get_kind(x) != PV_KIND_INVALID; }

pv pv_ref(pv);
void pv_unref(pv);

int pv_get_refcnt(pv);

int pv_equal(pv, pv);
int pv_identical(pv, pv);
int pv_contains(pv, pv);

pv pv_invalid(void);
pv pv_invalid_with_msg(pv);
pv pv_invalid_get_msg(pv);
int pv_invalid_has_msg(pv);

pv pv_null(void);
pv pv_true(void);
pv pv_false(void);
pv pv_bool(int);

pv pv_number(double);
double pv_number_value(pv);
int pv_is_integer(pv);

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

#ifdef __GNUC__
#define PV_PRINTF_LIKE(fmt_arg_num, args_num) \
  __attribute__ ((__format__( __printf__, fmt_arg_num, args_num)))
#define PV_VPRINTF_LIKE(fmt_arg_num) \
  __attribute__ ((__format__( __printf__, fmt_arg_num, 0)))
#endif


pv pv_string(const char*);
pv pv_string_sized(const char*, int);
pv pv_string_empty(int len);
int pv_string_length_bytes(pv);
int pv_string_length_codepoints(pv);
unsigned long pv_string_hash(pv);
const char* pv_string_value(pv);
pv pv_string_indexes(pv j, pv k);
pv pv_string_slice(pv j, int start, int end);
pv pv_string_concat(pv, pv);
pv pv_string_vfmt(const char*, va_list) PV_VPRINTF_LIKE(1);
pv pv_string_fmt(const char*, ...) PV_PRINTF_LIKE(1, 2);
pv pv_string_append_codepoint(pv a, uint32_t c);
pv pv_string_append_buf(pv a, const char* buf, int len);
pv pv_string_append_str(pv a, const char* str);
pv pv_string_split(pv j, pv sep);
pv pv_string_explode(pv j);
pv pv_string_implode(pv j);

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



int pv_get_refcnt(pv);

typedef void (*pv_nomem_handler_f)(void *);
void pv_nomem_handler(pv_nomem_handler_f, void *);

pv pv_get(pv, pv);
pv pv_set(pv, pv, pv);
pv pv_has(pv, pv);
pv pv_setpath(pv, pv, pv);
pv pv_getpath(pv, pv);
pv pv_delpaths(pv, pv);
pv pv_keys(pv /*object or array*/);
pv pv_keys_unsorted(pv /*object or array*/);
int pv_cmp(pv, pv);
pv pv_group(pv, pv);
pv pv_sort(pv, pv);

#ifdef __cplusplus
}
#endif

#endif


/*

true/false/null:
check kind

number:
introduce/eliminate pv
to integer

array:
copy
free
slice
index
update

updateslice?


 */
