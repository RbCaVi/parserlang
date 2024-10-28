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

import struct

types = {typ:i for i,typ in enumerate([
	'BLOCK',
	'DEFFUNC',
	'IF',
	'DEF',
	'RETURN',
	'SIG',
	'EXPR',
	'NUM',
	'SYM',
])}

def dump(stmt, indent = 'a:'):
	typ = stmt[0]
	data = struct.pack('<I', types[typ])
	if typ == 'BLOCK':
		es = [dump(e) for e in stmt[1:]]
		lens = [len(e) for e in es]
		for l in lens:
			data += struct.pack('<I', l)
		for e in es:
			data += e
	elif typ == 'DEFFUNC':
		_,name,sig,code = stmt
		e1 = dump(sig)
		e2 = dump(code)
		data += struct.pack("<III", len(name), len(e1), len(e2))
		data += name.encode('utf-8')
		data += e1
		data += e2
	elif typ == 'IF':
		_,cond,code = stmt
		e1 = dump(cond)
		e2 = dump(code)
		data += struct.pack("<II", len(e1), len(e2))
		data += e1
		data += e2
	elif typ == 'DEF':
		_,name,val = stmt
		data += struct.pack('<I', len(name))
		data += name.encode('utf-8')
		data += dump(val)
	elif typ == 'RETURN':
		_,val = stmt
		data += dump(val)
	elif typ == 'SIG':
		es = [e.encode() for e in stmt[1:]]
		lens = [len(e) for e in es]
		for l in lens:
			data += struct.pack('<I', l)
		for e in es:
			data += e
	elif typ == 'EXPR':
		op = stmt[1]
		data += opids[op]
		data += struct.pack('<I', len(stmt) - 2)
		es = [dump(e) for e in stmt[2:]]
		lens = [len(e) for e in es]
		for l in lens:
			data += struct.pack('<I', l)
		for e in es:
			data += e
	elif typ == 'NUM':
		num = stmt[1]
		data += struct.pack('<I', num)
	elif typ == 'SYM':
		sym = stmt[1]
		data += struct.pack('<I', len(sym))
		data += sym.encode('utf-8')
	else:
		raise ValueError(f'what??? ({stmt})')
	return data

for a,s in prgm(source):
	print(treeexpr(a))
	dump(a)
	print()