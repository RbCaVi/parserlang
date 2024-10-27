# pl parser
# converts pl source (.pls) to pl parsed tree (.plp)

import sys
filein,fileout = sys.argv[1],sys.argv[2]

lines = []
with open(filein) as f:
	source = f.read()


from stmt import stmts
from parsers import concat, transform
from parser import parser
from expr import treeexpr

@parser
def eof(s):
	if s.strip() == '':
		yield None,''

@transform(concat(stmts, eof))
def prgm(data):
	return data[0]

print(source)

for a,s in prgm(source):
	print(treeexpr(a))
	print()