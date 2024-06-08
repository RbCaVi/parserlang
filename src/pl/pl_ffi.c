#include "pl_ffi.h"

static pv_type ffi_type;

struct pl_ffi_entry {
  void *func; // function to call
  size_t nargs;
  pl_native_from_pv_f *argconvs;
  pl_pv_from_native_f retconv;
  ffi_cif cif;
};

static pl_ffi_entry *pl_ffi_entries=NULL;
static size_t pl_ffi_entries_count = 0, pl_ffi_entries_size = 0;

static size_t pl_add_ffi_entry(pl_ffi_data data, ffi_cif cif) {
  size_t idx = pl_ffi_entries_count;
  pl_ffi_entries_count++;
  if (pl_ffi_entries_count > pl_ffi_entries_size) {
    pl_ffi_entries_size = pl_ffi_entries_size * 1.5f;
    pl_ffi_entries = realloc(pl_ffi_entries,pl_ffi_entries_size * sizeof(pl_ffi_entry));
  }
  pl_ffi_entry *entry = pl_ffi_entries + idx;
  entry->func = data.func;
  entry->nargs = data.nargs;
  entry->argconvs = malloc(data.nargs * sizeof(pl_native_from_pv_f));
  if (entry->argconvs == NULL) {
    abort();
  }
  for (size_t i = 0; i < data.nargs; i++) {
    entry->argconvs[i] = data.argconvs[i].converter;
  }
  entry->retconv = data.retconv.converter;
  entry->cif = cif;
  return idx;
}

size_t pl_add_ffi_func(pl_ffi_data data){
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
  int idx = pl_add_ffi_entry(data,cif);
  return idx;
}

static int max(int a, int b) {
  if (a > b) {
    return a;
  }
  return b;
}

void pl_ffi_call(pl_state* state, size_t idx){
  // get func desc
  pl_ffi_entry entry = pl_ffi_entries[idx];
  // convert args
  void **data = malloc(entry.nargs * sizeof(void*));
  for (size_t i = 0; i < entry.nargs; i++) {
    pv val = pl_stack_pop(state->stack);
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
  ffi_call(entry.data.cif,FFI_FN(entry.func),retval,data);
  // convert return
  pv retpv;
  int status = entry.retconv.converter(retval,&retpv);
  if (status != 0) {
    abort();
  }
  // push return
  pl_stack_push(state->stack, retpv);
}