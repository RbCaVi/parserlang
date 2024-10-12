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

instrs = []
labels = {}
blen = 0

with open(filein) as f:
	it = iter(f)
	for line in it:
		line = line.split('//')[0]
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

with open(fileout, 'wb') as f:
	f.write(bytecode)