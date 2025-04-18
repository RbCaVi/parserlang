#define UOP(upper_name, lower_name, op) \
OP(upper_name, lower_name, #op, 1)
#define BOP(upper_name, lower_name, op, isdefault) \
OP(upper_name, lower_name, #op, 2)
#include "pv_number_ops_data.h"
#undef UOP
#undef BOP

OP(EQUAL, equal, "==", 2)
OP(CALL, call, "(", 0)
OP(ARRAY, array, "[]", 0)
OP(IDX, idx, "[", 0)
OP(GREATER, greater, ">", 2)
OP(LESS, less, "<", 2)
OP(GREATEREQUAL, greaterequal, ">=", 2)
OP(LESSEQUAL, lessequal, "<=", 2)
OP(NOTEQUAL, notequal, "!=", 2)
OP(NOT, not, "!", 1)