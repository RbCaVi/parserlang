def x = 2
if x == 1 then {
	x = 15
}
#yield x

def a = []
def b = [1]
def c = add(1, 2, 4)
a = 5
while a == 5 do {
	a = a + 15
}
for x in [1, 2] do {
	a = [a, x]
}
def d = concat(a, b)
yield ([c, b, a, d]) # chess battle advanced
return ([a, b, c, d]) # only one element arrays can be returned without parentheses - this is a limitation of the parser