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

//typedef char pv_kind; // 0-31

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

#ifdef __GNUC__
#define PV_PRINTF_LIKE(fmt_arg_num, args_num) \
  __attribute__ ((__format__( __printf__, fmt_arg_num, args_num)))
#define PV_VPRINTF_LIKE(fmt_arg_num) \
  __attribute__ ((__format__( __printf__, fmt_arg_num, 0)))
#endif

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
