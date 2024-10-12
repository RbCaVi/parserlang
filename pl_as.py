# pl assembler
# converts pl assembly (.pla) to pl bytecode (.plb)

import re
import struct

with open("src/pl/pl_opcodes_data.h") as f:
	opcodedata = f.read()

types = {
	"int": "i",
	"unsigned int": "I",
	"double": "d",
}

opcodes = {}
for i,(opcode,members) in enumerate(re.findall('OPCODE\\(\\s*(\\S*)\\s*,\\s*(\\{.*\\})\\s*\\)', opcodedata)):
	fmt = ''
	for typ in re.findall('([a-zA-Z_0-9\\s]+)\\s[a-zA-Z_0-9]+;', members):
		typ = ' '.join(typ.split())
		fmt += types[typ]
	opcodes[opcode] = i,fmt

jumpopcodes = ["JUMP", "JUMPIF", "ITERATE"]

import sys
filein,fileout = sys.argv[1],sys.argv[2]

lines = []
with open(filein) as f:
	for line in f:
		line = line.split('//')[0]
		if line.strip() == '':
			continue
		lines.append(line)

def assemble(lines):
	instrs = []
	labels = {}
	blen = 0

	it = iter(lines)
	for line in it:
		*lbls,line = line.split(':')
		for lbl in lbls:
			label, = lbl.split()
			labels[label] = blen
		if line.strip() == '':
			continue
		else:
			op,*args = line.split()
			op = op.upper()
			blen += struct.calcsize('<I' + opcodes[op][1])
			instrs.append((op, args, blen))

	bytecode = b''
	for op,args,bpos in instrs:
		if op in jumpopcodes:
			label, = args
			args = [labels[label] - bpos]
		i,fmt = opcodes[op]
		args = [(float if typ == 'd' else int)(arg) for typ,arg in zip(fmt,args)]
		bytecode += struct.pack('<I' + opcodes[op][1], i, *args)

	return bytecode

def parse(lines):
	g = []
	#main = None
	v = {}
	it = iter(lines)
	for line in it:
		op,*args = line.split()
		if op == 'def':
			v[args[0]] = args[1:]
		elif op == 'defglobals':
			g = args
		elif op == 'defmain':
			main = args[0]
		elif op == 'func':
			flines = []
			line = next(it)
			while line.split()[0] != 'endfunc':
				flines.append(line)
				line = next(it)
			v[args[0]] = ['func', flines]
		else:
			print('???',line)
	return g,main,v

print(parse(lines))