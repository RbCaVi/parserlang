import sys
filein = sys.argv[1]

lines = []
with open(filein, 'rb') as f:
	data = f.read()

import struct

maini,glen,vlen = struct.unpack('<III', data[:struct.calcsize('<III')])
g = struct.unpack(f'<{glen}I', data[struct.calcsize('<III'):struct.calcsize('<III') + struct.calcsize(f'<{glen}I')])
v = struct.unpack(f'<{vlen}I', data[struct.calcsize(f'<III{glen}I'):struct.calcsize(f'<III{glen}I') + struct.calcsize(f'<{vlen}I')])

ntype = 0
atype = 1
ftype = 2

def getvar(i):
	vdata = data[v[i]:]
	typ, = struct.unpack('<I', vdata[:struct.calcsize('<I')])
	if typ == ntype:
		_typ,val = struct.unpack('<Id', vdata[:struct.calcsize('<Id')])
		return val
	elif typ == atype:
		_typ,alen = struct.unpack('<II', vdata[:struct.calcsize('<II')])
		a = struct.unpack(f'<{alen}I', vdata[struct.calcsize('<II'):struct.calcsize('<II') + struct.calcsize(f'<{alen}I')])
		return [getvar(i) for i in a]
	elif typ == ftype:
		_typ,flen = struct.unpack('<II', vdata[:struct.calcsize('<II')])
		return vdata[struct.calcsize('<II'):struct.calcsize('<II') + flen]
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