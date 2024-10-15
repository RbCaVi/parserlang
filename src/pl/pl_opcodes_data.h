// like jq with its OP() redefinition (no include guards)
/*
DUP duplicate top
ITER addr step the top of the stack and push the result or jump to const
JUMP addr jump unconditional
JUMPIF addr jump to const if top is true
POP delete top of stack
GETGLOBAL n get the nth value in the global table
DUPN n duplicate item n below to top
// set get and del will have path and constant versions
SET n pop k,v and set [k]=v on item n below
DEL n pop k and delete [k] on item n below
GET n pop k and get [k] on item n below
BACKTRACK addr save here and do the same as iter
MAKEREF n make a reference from the top n elements
CALLN n pop n arguments and a function, put the arguments into a stack frame, and call the function
CALL pop one number as n and act like CALLN
RET n return n values
// let someone else handle global switching? files are merged anyway
SETGLOBAL set a global var (constants too)
CALLCONST addr
CALLCONSTN addr n
*/

// each definition is OPCODE(opcode name, data members (as struct))

OPCODE(DUP, {}) // duplicate top of stack (= DUPN -n)
OPCODE(DUPN, {int n;}) // duplicate n below
OPCODE(POP, {}) // delete top of stack
OPCODE(SWAPN, {int n;}) // swap top with n below (= SWAPNN n -1)
OPCODE(SWAPNN, {int n1;int n2;}) // swap n1 below with n2 below

OPCODE(PUSHNULL, {}) // push null (literally just that)
OPCODE(PUSHBOOL, {int v;}) // push a bool with value v (0 or 1)
OPCODE(PUSHINT, {int n;}) // push an int with value n
OPCODE(PUSHDOUBLE, {double n;}) // push a double with value n
OPCODE(PUSHARRAY, {}) // push an empty array
OPCODE(PUSHOBJECT, {}) // push an empty object
OPCODE(PUSHGLOBAL, {int i;}) // push entry i in the global table

OPCODE(MAKEARRAY, {unsigned int n;}) // create an array with the top n values
OPCODE(APPENDA, {}) // pop a value and append it to the array below it
OPCODE(CONCATA, {}) // pop a value and append it to the array below it
OPCODE(SETA, {}) // pop a idx and value and set them on the array below them
OPCODE(SETAI, {int i;}) // pop a value and set it at index i on the array below it
OPCODE(GETA, {}) // pop a idx and array and push array[idx]
OPCODE(GETAI, {int i;}) // pop an array and push array[i]
OPCODE(LENA, {}) // pop an array and push the array's length
OPCODE(SLICEA, {}) // pop two idxs and an array and push the array but sliced from those two indexes
OPCODE(SLICEAL, {int i;}) // pop an array and push array[:i]
OPCODE(SLICEAM, {int i;}) // pop an array and push array[i:]
OPCODE(SLICEAR, {int i;}) // pop an array and push array[-i:]
OPCODE(SLICEAII, {int i1;int i2;}) // pop an array and push array[i1:i2]

OPCODE(MAKEOBJECT, {unsigned int n;}) // create an object with the top 2n values
OPCODE(SETO, {}) // pop a key and value and set them on the object below them
OPCODE(APPENDO, {}) // pop a key and value and set them on the object below them (error if the object already has the key)
OPCODE(GETO, {}) // pop a key and object and push object[key]
OPCODE(DELO, {}) // pop a key and delete it on the object below it
OPCODE(HASO, {}) // pop a key and object and push key in object
OPCODE(LENO, {}) // pop an object and push the object's length

OPCODE(CALL, {int n;}) // call n + 1 below with n arguments above it

#define UOP(upper_name, lower_name, expr) \
OPCODE(upper_name, {})
#define BOP(upper_name, lower_name, expr, isdefault) \
OPCODE(upper_name, {})
#include "pv_number_ops_data.h"
#undef UOP
#undef BOP

OPCODE(JUMP, {int target;}) // unconditional jump - target is bytes relative to the next instruction
OPCODE(JUMPIF, {int target;}) // pop one value and jump if it is true (it must be a boolean) - target is bytes relative to the next instruction

OPCODE(ITER, {}) // create an iterator (values for array, keys for object)
OPCODE(ITERK, {}) // create a keys iterator
OPCODE(ITERV, {}) // create a values iterator
OPCODE(ITERE, {}) // create a entries iterator
OPCODE(ITERATE, {int target;}) // pop an iterator, step it, and push the stepped iterator and result or jump to target (bytes relative to the next instruction)

OPCODE(RET, {}) // return one value (in generator, same as YIELD + GRET)
OPCODE(YIELD, {}) // return one value
OPCODE(GRET, {}) // return from a generator (no return value)
