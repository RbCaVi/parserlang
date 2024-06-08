#include "stack.h"
#include "pv.h"

// pop value
// convert to c
// append to list

enum pv_converter_code{
  CONVERT_OK=0,
  CONVERT_ERR
}

// return code
// expected to allocate a void* and store it in the second argument
typedef pv_converter_f pv_converter_code(*)(pv,void**)

