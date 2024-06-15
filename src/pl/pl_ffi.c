#include "pl/pl_ffi.h"

#include <stdlib.h>
#include "pl/util_pl.h"

typedef struct {
  void *func; // function to call
  size_t nargs;
  pl_native_from_pv *argconvs;
  pl_pv_from_native retconv;
  ffi_cif cif;
} pl_ffi_entry;

static pl_ffi_entry *pl_ffi_entries=NULL;
static size_t pl_ffi_entries_count = 0, pl_ffi_entries_size = 0;

static size_t add_ffi_entry(pl_ffi_data data, ffi_cif cif) {
  size_t idx;
  inc_size(idx,pl_ffi_entries,pl_ffi_entries_count,0,pl_ffi_entries_size,(size_t)((float)pl_ffi_entries_size * 1.5f));
  pl_ffi_entry *entry = pl_ffi_entries + idx;
  entry->func = data.func;
  entry->nargs = data.nargs;
  entry->argconvs = malloc(data.nargs * sizeof(pl_native_from_pv_f));
  if (entry->argconvs == NULL) {
    abort();
  }
  for (size_t i = 0; i < data.nargs; i++) {
    entry->argconvs[i] = data.argconvs[i];
  }
  entry->retconv = data.retconv;
  entry->cif = cif;
  return idx;
}

size_t pl_add_ffi_func(pl_ffi_data data) {
  ffi_cif cif;
  ffi_type **atypes = malloc(data.nargs * sizeof(ffi_type*));
  if (atypes == NULL) {
    abort();
  }
  for (size_t i = 0; i < data.nargs; i++) {
    atypes[i] = data.argconvs[i].type;
  }
  int status = ffi_prep_cif(&cif, FFI_DEFAULT_ABI, data.nargs, data.retconv.type, atypes);
  if (status != FFI_OK) {
    abort();
  }
  int idx = add_ffi_entry(data,cif);
  return idx;
}

void pl_ffi_call(pl_state *state, size_t idx){
  // get func desc
  pl_ffi_entry entry = pl_ffi_entries[idx];
  // convert args
  void **data = malloc(entry.nargs * sizeof(void*));
  for (size_t i = 0; i < entry.nargs; i++) {
    pv val = pl_stack_top(state->stack);
    state->stack = pl_stack_top(state->stack);
    data[i] = malloc(entry.argconvs[i].type->size);
    if (data[i] == NULL) {
      abort();
    }
    int status = entry.argconvs[i].converter(val,data[i]);
    if (status != 0) {
      abort();
    }
  }
  // call
  void *retval = malloc(max(entry.retconv.type->size,sizeof(long)));
  ffi_call(&entry.cif,FFI_FN(entry.func),retval,data);
  // convert return
  pv retpv;
  int status = entry.retconv.converter(retval,&retpv);
  if (status != 0) {
    abort();
  }
  // push return
  pl_stack_push(state->stack, retpv);
}