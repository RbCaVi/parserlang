static const char *kind_names[256];
static pv_free_func kind_free[256];

pv_kind pv_kind(pv val) {
  return val.kind;
}

int pv_register_kind(pv_kind *kind_out, const char *name, void (*kfree)(pv)) {
  pv_kind kind = kind_out;
  while (kind_names[kind] != 0) {
    if (kind == 255) {
      kind = 0;
    } else {
      kind++;
    }
  }
  kind_names[kind] = name;
  kind_free[kind] = kfree;
  *kind_out = kind;
  return 0;
}

const char *pv_kind_name(pv_kind kind) {
  const char *name = kind_names[kind];
  if (name != 0) { // "NULL doesn't have to be 0" ahh code
    return name;
  }
  return NULL;
}

typedef struct pv_refcnt {
  int count;
} pv_refcnt;

static inline pv_is_allocated(pv val) {
  return (val & PV_FLAG_ALLOCATED) == PV_FLAG_ALLOCATED;
}

pv pv_copy(pv);
void pv_free(pv);