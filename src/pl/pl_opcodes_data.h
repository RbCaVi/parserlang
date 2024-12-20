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

#ifndef JOPCODE
#define JOPCODE OPCODE
#define _NO_JOPCODE
#endif

// each definition is OPCODE(opcode name, data members (as struct))

OPCODE(DUP, dup, {}) // duplicate top of stack (= DUPN -1)
OPCODE(DUPN, dupn, {int n;}) // duplicate n below
OPCODE(POP, pop, {}) // delete top of stack
OPCODE(POPTO, popto, {unsigned int n;}) // pop elements until locals is n elements tall
OPCODE(SWAPN, swapn, {int n;}) // swap top with n below (= SWAPNN n -1)
OPCODE(SWAPNN, swapnn, {int n1;int n2;}) // swap n1 below with n2 below

OPCODE(PUSHNULL, pushnull, {}) // push null (literally just that)
OPCODE(PUSHBOOL, pushbool, {int v;}) // push a bool with value v (0 or 1)
OPCODE(PUSHINT, pushint, {int n;}) // push an int with value n
OPCODE(PUSHDOUBLE, pushdouble, {double n;}) // push a double with value n
OPCODE(PUSHARRAY, pusharray, {}) // push an empty array
OPCODE(PUSHOBJECT, pushobject, {}) // push an empty object

OPCODE(PUSHGLOBAL, pushglobal, {int i;}) // push entry i in the global table
//OPCODE(SETGLOBAL, setglobal, {int i;}) // pop from the stack and set entry i in the global table // should only be used in the global scope before calling functions

OPCODE(MAKEARRAY, makearray, {unsigned int n;}) // create an array with the top n values
OPCODE(APPENDA, appenda, {}) // pop a value and append it to the array below it
OPCODE(CONCATA, concata, {}) // pop a value and append it to the array below it
OPCODE(SETA, seta, {}) // pop a idx and value and set them on the array below them
OPCODE(SETAI, setai, {int i;}) // pop a value and set it at index i on the array below it
OPCODE(GETA, geta, {}) // pop a idx and array and push array[idx]
OPCODE(GETAI, getai, {int i;}) // pop an array and push array[i]
OPCODE(LENA, lena, {}) // pop an array and push the array's length
OPCODE(SLICEA, slicea, {}) // pop two idxs and an array and push the array but sliced from those two indexes
OPCODE(SLICEAL, sliceal, {int i;}) // pop an array and push array[:i]
OPCODE(SLICEAM, sliceam, {int i;}) // pop an array and push array[i:]
OPCODE(SLICEAR, slicear, {int i;}) // pop an array and push array[-i:]
OPCODE(SLICEAII, sliceaii, {int i1;int i2;}) // pop an array and push array[i1:i2]

OPCODE(MAKEOBJECT, makeobject, {unsigned int n;}) // create an object with the top 2n values
OPCODE(SETO, seto, {}) // pop a key and value and set them on the object below them
OPCODE(APPENDO, appendo, {}) // pop a key and value and set them on the object below them (error if the object already has the key)
OPCODE(GETO, geto, {}) // pop a key and object and push object[key]
OPCODE(DELO, delo, {}) // pop a key and delete it on the object below it
OPCODE(HASO, haso, {}) // pop a key and object and push key in object
OPCODE(LENO, leno, {}) // pop an object and push the object's length

OPCODE(CALL, call, {int n;}) // call n + 1 below with n arguments above it

#define UOP(upper_name, lower_name, expr) \
OPCODE(upper_name, lower_name, {})
#define BOP(upper_name, lower_name, expr, isdefault) \
OPCODE(upper_name, lower_name, {})
#include "pv_number_ops_data.h"
#undef UOP
#undef BOP

OPCODE(EQUAL, equal, {})

JOPCODE(JUMP, jump, {int target;}) // unconditional jump - target is bytes relative to the next instruction
JOPCODE(JUMPIF, jumpif, {int target;}) // pop one value and jump if it is true (it must be a boolean) - target is bytes relative to the next instruction

OPCODE(ITER, iter, {}) // create an iterator (values for array, keys for object)
OPCODE(ITERK, iterk, {}) // create a keys iterator
OPCODE(ITERV, iterv, {}) // create a values iterator
OPCODE(ITERE, itere, {}) // create a entries iterator
JOPCODE(ITERATE, iterate, {int target;}) // pop an iterator, step it, and push the stepped iterator and result or jump to target (bytes relative to the next instruction)

OPCODE(RET, ret, {}) // return one value (ends a normal function)
OPCODE(GRET, gret, {}) // return from a generator or return pv_invalid

#ifdef _NO_JOPCODE
#undef JOPCODE
#endif