UOP(NEG, neg, -)

#define no(x)
#define ok(x) x

BOP(ADD, add, +, ok)
BOP(SUB, sub, -, ok)
BOP(MUL, mul, *, ok)
BOP(DIV, div, /, no)
BOP(IDIV, idiv, , no)
BOP(MOD, mod, %, no)
