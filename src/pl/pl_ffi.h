#include <stddef.h>
#include "pl_stack.h"
#include "pv.h"
#include <ffi.h>

// pop value
// convert to c
// append to list

// return code
// returns value in second argument
// used for ffi calls

typedef int (*pl_native_from_pv_f)(pv,void*);

typedef struct {
  pl_native_from_pv_f converter;
  ffi_type *type;
} pl_native_from_pv;

typedef int (*pl_pv_from_native_f)(void*,pv*);

typedef struct {
  pl_pv_from_native_f converter;
  ffi_type *type;
} pl_pv_from_native;

typedef struct {
  void *func; // function to call
  size_t nargs;
  pl_native_from_pv *argconvs;
  pl_pv_from_native retconv;
} pl_ffi_data;

size_t pl_add_ffi_func(pl_ffi_data data);
void pl_ffi_call(pl_state*, size_t);