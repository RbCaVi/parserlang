import struct

bopcodes={
  '+':'ADD',
}

uopcodes={
  '-':'NEG',
}

instructions={
  'DUP':((),1,2),
  'ITER':(('addroff',),1,2),
  'JUMP':(('addroff',),0,0), # step to the next inst, then jump bytes
  'JUMPF':(('addroff',),1,0),
  'JUMPT':(('addroff',),1,0),
  'POP':((),1,0),
  'PUSHCONST':(('num',),0,1),
  'DUPN':(('num',),0,1),
  'SETPATH':(('num',),2,0),
  'DELPATH':(('num',),1,0),
  'GETPATH':(('num',),1,1),
  'SETPATHCONST':(('num',),1,0),
  'DELPATHCONST':(('num',),0,0),
  'GETPATHCONST':(('num',),0,1),
  'BACKTRACK':(('addr'),1,2),
  'MAKEREF':(('num',),0,1),
  'CALLN':(('num'),'ivar',1),
  'CALL':((),'svar',1),
  'RET':((),1,'return'),
  'STORECONST':((),1,0),
  'CALLCONSTADDR':(('addr'),'svar',1),
  'CALLCONSTADDRN':(('addr','num'),'ivar',1),
  'CALLCONSTF':(('num'),'svar',1),
  'CALLCONSTFN':(('num','num'),'ivar',1),  'RESETSTACK':(('num',),'ivar',0),
  'PUSHNUM':(('num',),0,1),
  'PUSHNUM32':(('num32',),0,1),
  'PUSHNUM64':(('num64',),0,1),
  'PUSHARRAY':((),0,1),
}

for opcode in bopcodes.values():
  instructions[opcode]=((),2,1)

for opcode in uopcodes.values():
  instructions[opcode]=((),1,1)

opnames=dict(enumerate(instructions))
opcodes=dict(map(reversed,opnames.items()))

# 8 bit units

sizes={
  'opcode':1,
  'addr':2,
  'addroff':2,
  'num':2,
  'num32':4,
  'num64':8,
  'unum':2,
}

formats={
  'opcode':'<B',
  'addr':'<h',
  'addroff':'<h',
  'num':'<h',
  'num32':'<i',
  'num64':'<q',
  'unum':'<H',
}


def opsize(opcode):
  size=sizes['opcode']
  for param in instructions[opcode][0]:
    size+=sizes[param]
  return size

def formatparam(ptype,param):
  return struct.pack(formats[ptype],param)

def inst(opname,*params):
  inst=formatparam('opcode',opcodes[opname])
  paramtypes=instructions[opname][0]
  assert len(paramtypes)==len(params)
  for ptype,param in zip(paramtypes,params):
    inst+=formatparam(ptype,param)
  return inst