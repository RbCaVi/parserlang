#ifndef PV_H
#define PV_H

// builtin types:
// invalid
// bool
// int / float
// array
// string
// map / object

// pl types:
// func
// native func
// closure
// generator

typedef unsigned char pv_kind;

struct pv_refcnt;

// stripped version of jv
// please do not touch flags and data
// only read kind
typedef struct {
  pv_kind kind;
  unsigned char flags;
  union {
    struct pv_refcnt* data; // may be kind specific data instead of a pointer if flags does not contain PV_FLAG_ALLOCATED
    double _aaaaaa_;
    int _aaaaaaaaa_;
  };
} pv;

typedef void (*pv_free_func)(pv);

pv_kind pv_get_kind(pv);
int pv_register_kind(pv_kind*, const char*, pv_free_func);
const char *pv_kind_name(pv_kind);

#define PV_FLAG_ALLOCATED 0x01
#define PV_IS_ALLOCATED(val) (((val).flags & PV_FLAG_ALLOCATED) == PV_FLAG_ALLOCATED)

pv pv_copy(pv);
void pv_free(pv);
int pv_get_refcount(pv);

pv pv_invalid();

#endif