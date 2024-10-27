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

arities = {
	('==', 2),
	('+', 2),
	('-', 2),
	('-', 1),
}

def dump(stmt, indent = 'a:'):
	typ = stmt[0]
	styp = stmt[1]
	print(indent, typ)
	if typ == 'BLOCK':
		for s in stmt[1:]:
			dump(s, '  ' + indent)
	elif typ == 'DEFFUNC':
		_,name,sig,code = stmt
		print("  " + indent, name)
		dump(sig, '  ' + indent)
		dump(code, '  ' + indent)
	elif typ == 'IF':
		_,cond,code = stmt
		dump(cond, '  ' + indent)
		dump(code, '  ' + indent)
	elif typ == 'DEF':
		_,vtype,name,val = stmt
		print("  " + indent, vtype, name)
		dump(val, '  ' + indent)
	elif typ == 'RETURN':
		_,val = stmt
		dump(val, '  ' + indent)
	elif typ == 'SIG':
		for s in stmt[1:]:
			dump(s, '  ' + indent)
	elif typ == 'ARG':
		_,atype,name = stmt
		print("  " + indent, atype, name)
	elif typ == 'EXPR':
		print(indent, styp)
		if styp == '(':
			for e in stmt[2:]:
				dump(e, '  ' + indent)
		else:
			assert (styp, len(stmt) - 2) in arities
			for e in stmt[2:]:
				dump(e, '  ' + indent)
	elif typ == 'NUM':
		print(indent, styp)
		pass
	elif typ == 'SYM':
		print(indent, styp)
		pass
	else:
		print(indent, styp)
		print("  " + indent, "???")

for a,s in prgm(source):
	print(treeexpr(a))
	dump(a)
	print()