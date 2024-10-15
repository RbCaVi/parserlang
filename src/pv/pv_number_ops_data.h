UOP(NEG, neg, -n)

#define no(x)
#define ok(x) x

BOP(ADD, add, n1 + n2, ok)
BOP(SUB, sub, n1 - n2, ok)
BOP(MUL, mul, n1 * n2, ok)
BOP(DIV, div, n1 / n2, no)
BOP(IDIV, idiv, n1 / n2, no)
BOP(MOD, mod, n1 % n2, no)
