import sys
filein = sys.argv[1]

with open(filein, 'rb') as f:
	data = f.read()

import struct

def unpackstart(fmt, data):
	return struct.unpack(fmt,data[:struct.calcsize(fmt)]),data[struct.calcsize(fmt):]

hdata = data
(maini,glen,vlen),hdata = unpackstart('<III', hdata)
g,hdata = unpackstart(f'<{glen}I', hdata)
v,hdata = unpackstart(f'<{vlen}I', hdata)

ntype = 0
atype = 1
ftype = 2

def getvar(i):
	vdata = data[v[i]:]
	(typ,),vdata = unpackstart('<I', vdata)
	if typ == ntype:
		(val,),_ = unpackstart('<d', vdata)
		return val
	elif typ == atype:
		(alen,),vdata = unpackstart('<I', vdata)
		a,_ = unpackstart(f'<{alen}I', vdata)
		return [getvar(i) for i in a]
	elif typ == ftype:
		(flen,),_ = unpackstart('<I', vdata)
		return vdata[:flen]
	else:
		return None

print('main bytecode:')
print(f'  {maini}: {getvar(maini)}')
print()
print('vars:')
for pos in range(vlen):
	print(f'  {v[pos]}: {getvar(pos)}')
print()
print('globals:')
for i in range(glen):
	print(f'  {g[i]}: {getvar(g[i])}')