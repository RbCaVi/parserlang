#define UOP(upper_name, lower_name, op) \
OP(op, 1)
#define BOP(upper_name, lower_name, op, isdefault) \
OP(op, 2)
#include "pv_number_ops_data.h"
#undef UOP
#undef BOP

OP(==, 2)
OP((, 0)