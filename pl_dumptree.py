# dump a .plp file

import re
import subprocess

opcodedata = subprocess.check_output(["gcc", "-E", "-I", "src/pv", "-I", "src/pl", "src/plc/plc_op_ids.h", "-o", "-"], encoding = 'utf-8')

opids = {}
for i,(op,arity) in enumerate(re.findall('OP\\(\\s*"(\\S*)"\\s*,\\s*(\\S+)\\s*\\)', opcodedata)):
	opids[i] = op

types = [
	'BLOCK',
	'DEFFUNC',
	'IF',
	'DEF',
	'RETURN',
	'SIG',
	'EXPR',
	'NUM',
	'SYM',
]

import struct

def unpackstart(fmt, data):
	return struct.unpack(fmt,data[:struct.calcsize(fmt)]),data[struct.calcsize(fmt):]

def undump(data):
	(typeid,),data = unpackstart('<I', data)
	typ = types[typeid]
	out = [typ]
	if typ == 'BLOCK':
		(n,),data = unpackstart('<I', data)
		lens,data = unpackstart(f'<{n}I', data)
		for l in lens:
			e,data = data[:l],data[l:]
			out.append(undump(e))
	elif typ == 'DEFFUNC':
		(nl,sl,cl),data = unpackstart(f'<III', data)
		name,data = data[:nl],data[nl:]
		out.append(name.decode('utf-8'))
		sig,data = data[:sl],data[sl:]
		out.append(undump(sig))
		code,data = data[:cl],data[cl:]
		out.append(undump(code))
	elif typ == 'IF':
		(bl,cl),data = unpackstart(f'<II', data)
		cond,data = data[:bl],data[bl:]
		out.append(undump(cond))
		code,data = data[:cl],data[cl:]
		out.append(undump(code))
	elif typ == 'DEF':
		(nl,),data = unpackstart(f'<I', data)
		name,val = data[:nl],data[nl:]
		out.append(name.decode('utf-8'))
		out.append(undump(val))
	elif typ == 'RETURN':
		out.append(undump(data))
	elif typ == 'SIG':
		(n,),data = unpackstart('<I', data)
		lens,data = unpackstart(f'<{n}I', data)
		for l in lens:
			arg,data = data[:l],data[l:]
			out.append(arg.decode('utf-8'))
	elif typ == 'EXPR':
		(opid,n),data = unpackstart('<II', data)
		op = opids[opid]
		out.append(op)
		lens,data = unpackstart(f'<{n}I', data)
		for l in lens:
			e,data = data[:l],data[l:]
			out.append(undump(e))
	elif typ == 'NUM':
		(n,),data = unpackstart('<I', data)
		out.append(n)
	elif typ == 'SYM':
		(nl,),data = unpackstart(f'<I', data)
		name,data = data[:nl],data[nl:]
		out.append(name.decode('utf-8'))
	else:
		print(typ)
	return out

import sys
filein = sys.argv[1]

with open(filein, 'rb') as f:
	data = f.read()

print(undump(data))