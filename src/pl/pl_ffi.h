#include "pl_stack.h"
#include "pv.h"
#include <ffi/ffi.h>

// pop value
// convert to c
// append to list

// return code
// returns value in second argument
// used for ffi calls

typedef int (*pl_native_from_pv_f)(pv,void*);

struct pl_native_from_pv {
  pl_native_from_pv_f converter;
  ffi_type *type;
};

typedef int (*pl_pv_from_native_f)(void*,pv*);

struct pl_pv_from_native {
  pl_pv_from_native_f converter;
  ffi_type *type;
};

struct pl_ffi_data {
  void *func; // function to call
  size_t nargs;
  pl_native_from_pv *argconvs;
  pl_pv_from_native retconv;
}

int pl_add_ffi_func(void*,pl_native_from_pv[],pl_pv_from_native);
void pl_ffi_call(pl_state*, size_t)