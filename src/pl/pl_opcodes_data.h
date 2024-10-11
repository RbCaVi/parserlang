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

OPCODE(DUP,{}) // duplicate top of stack
OPCODE(PUSHNUM,{double n;}) // push a number with value n
OPCODE(SWAPN,{int n;}) // swap top with n below
OPCODE(PUSHGLOBAL,{int i;}) // push entry i in the global table
OPCODE(CALL,{int n;}) // call n below with arguments above it
OPCODE(RET,{}) // return one value
OPCODE(ADD,{}) // pop two values, add them, and push the result
