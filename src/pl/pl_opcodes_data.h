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
OPCODE(DUPN, dupn, {int n;}) // duplicate stack[n]
OPCODE(SETN, setn, {int n;}) // pop and set stack[n]
OPCODE(POP, pop, {}) // delete top of stack
OPCODE(POPTO, popto, {unsigned int n;}) // pop elements until locals is n elements tall
OPCODE(SWAPN, swapn, {int n;}) // swap top with stack[n] (= SWAPNN n -1)
OPCODE(SWAPNN, swapnn, {int n1;int n2;}) // swap stack[n1] with stack[n2]

OPCODE(PUSHNULL, pushnull, {}) // push null (literally just that)
OPCODE(PUSHBOOL, pushbool, {int v;}) // push a bool with value v (0 or 1)
OPCODE(PUSHINT, pushint, {int n;}) // push an int with value n
OPCODE(PUSHDOUBLE, pushdouble, {double n;}) // push a double with value n
OPCODE(PUSHARRAY, pusharray, {}) // push an empty array
OPCODE(PUSHOBJECT, pushobject, {}) // push an empty object

OPCODE(PUSHGLOBAL, pushglobal, {int i;}) // push entry i in the global table
//OPCODE(SETGLOBAL, setglobal, {int i;}) // pop from the stack and set entry i in the global table // should only be used in the global scope before calling functions

// constructors
OPCODE(MAKESTRING, makestring, {}) // create an empty string
OPCODE(MAKEARRAY, makearray, {unsigned int n;}) // create an array with the top n values
OPCODE(MAKEOBJECT, makeobject, {unsigned int n;}) // create an object with the top 2n keys and values (alternating)

// object only
OPCODE(APPENDO, appendo, {}) // like SET, but asserts the key does not exist
OPCODE(DELO, delo, {}) // pop a key and delete it on the object below it
OPCODE(HASO, haso, {}) // pop a key and object and push key in object

// string / array
OPCODE(APPEND, append, {}) // pop v and x and push x.append(v)
OPCODE(CONCAT, concat, {}) // pop x1 and x2 and push concat(x1, x2)
OPCODE(SETI, seti, {int i;}) // pop v and x and push x[i] = v
OPCODE(GETI, geti, {int i;}) // pop x and push x[i]
OPCODE(SLICE, slice, {}) // pop idx1, idx2, and x and push x[idx1:idx2]
OPCODE(LEFT, left, {}) // pop idx and x and push x[:idx]
OPCODE(MID, mid, {}) // pop idx and x and push x[idx:]
OPCODE(RIGHT, right, {}) // pop idx and x and push x[-idx:]
OPCODE(LEFTI, lefti, {int i;}) // pop x and push x[:i]
OPCODE(MIDI, midi, {int i;}) // pop x and push x[i:]
OPCODE(RIGHTI, righti, {int i;}) // pop x and push x[-i:]
OPCODE(SLICEII, sliceii, {int i1;int i2;}) // pop x and push x[i1:i2]

// string / array / object
OPCODE(LEN, len, {}) // pop a x and push len(x)
OPCODE(SET, set, {}) // pop a idx / key and value and set them on the string, array, or object below them
OPCODE(GET, get, {}) // pop a idx / key and string, array, or object and push the value at idx in that string, array, or object

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
JOPCODE(JUMPIFNOT, jumpifnot, {int target;}) // pop one value and jump if it is false (it must be a boolean) - target is bytes relative to the next instruction

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