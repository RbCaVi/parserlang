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
	print(indent, typ, styp)
	if typ == 'STMT':
		if styp == 'many':
			for s in stmt[2:]:
				dump(s, '  ' + indent)
		elif styp == 'func':
			_,__,name,sig,code = stmt
			print("  " + indent, name)
			dump(sig, '  ' + indent)
			dump(code, '  ' + indent)
		elif styp == 'if':
			_,__,cond,code = stmt
			dump(cond, '  ' + indent)
			dump(code, '  ' + indent)
		elif styp == 'def':
			_,__,vtype,name,val = stmt
			print("  " + indent, vtype, name)
			dump(val, '  ' + indent)
		elif styp == 'return':
			_,__,val = stmt
			dump(val, '  ' + indent)
		else:
			print("  " + indent, "???")
	elif typ == 'SIG':
		assert styp == 'sig'
		for s in stmt[2:]:
			dump(s, '  ' + indent)
	elif typ == 'ARG':
		_,atype,name = stmt
		print("  " + indent, atype, name)
	elif typ == 'EXPR':
		if styp == '(':
			for e in stmt[2:]:
				dump(e, '  ' + indent)
		else:
			assert (styp, len(stmt) - 2) in arities
			for e in stmt[2:]:
				dump(e, '  ' + indent)
	elif typ == 'NUM':
		pass
	elif typ == 'SYM':
		pass
	else:
		print("  " + indent, "???")

for a,s in prgm(source):
	print(treeexpr(a))
	dump(a)
	print()