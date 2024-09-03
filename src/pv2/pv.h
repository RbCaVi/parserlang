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
// please do not touch
typedef struct {
  pv_kind kind;
  unsigned char flags;
  struct pv_refcnt* ptr; // may be kind specific data instead of a pointer if flags does not contain PV_FLAG_ALLOCATED
} pv;

pv_kind pv_kind(pv);
int pv_register_kind(const char*, pv_kind*);
const char *pv_kind_name(pv_kind);

#define PV_FLAG_ALLOCATED 0x01

pv pv_copy(pv);
void pv_free(pv);