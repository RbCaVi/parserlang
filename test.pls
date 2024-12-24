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
yield ([c, b, a]) # chess battle advanced
return ([a, b, c]) # only one element arrays can be returned without parentheses - this is a limitation of the parser