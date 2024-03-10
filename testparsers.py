from parser import printresults
from parsers import strs,alternate,concat

def test(p,ss):
	for s in ss:
		print(f'parsing {repr(s)} with {repr(p)}')
		printresults(p(s))
		print()

p=strs(['6','5','65'])

test(p,['6','65','5'])

p1=strs(['7'])
p2=strs(['5'])
p=concat(p1,p2)

test(p,['75','7','5','77','55'])

p=alternate(p1,p2)

test(p,['75','7','5'])