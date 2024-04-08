from opcodes import opsize,opnames,instructions,formats,sizes
import struct

def hexlen(n,l):
  return hex(n)[2:].rjust(l,'0')

def disasm(insts):
  lines=[]
  i=0
  while i<len(insts):
    opcode,=struct.unpack(formats['opcode'],insts[i:i+sizes['opcode']])
    line=hexlen(i,6)
    i+=sizes['opcode']
    line+=' '
    line+=opnames[opcode].ljust(10)
    for ptype in instructions[opnames[opcode]][0]:
      param,=struct.unpack(formats[ptype],insts[i:i+sizes[ptype]])
      i+=sizes[ptype]
      line+=f' {hexlen(param,sizes[ptype]*2)}'
    lines.append(line)
  return '\n'.join(lines)

def disasm2(insts):
  lines=[]
  labels=dict()
  i=0
  labelcount=0
  while i<len(insts):
    opcode,=struct.unpack(formats['opcode'],insts[i:i+sizes['opcode']])
    line=(i,)
    i+=sizes['opcode']
    line+=(opnames[opcode],)
    for ptype in instructions[opnames[opcode]][0]:
      param,=struct.unpack(formats[ptype],insts[i:i+sizes[ptype]])
      i+=sizes[ptype]
      line+=((ptype,param,sizes[ptype],),)
      if ptype=='addr':
        addr=param
        if addr not in labels:
          labels[param]='f'+str(labelcount)
          labelcount+=1
      if ptype=='addroff':
        #print('s',i,opnames[opcode],param)
        addr=i+param
        if addr not in labels:
          labels[addr]='j'+str(labelcount)
          labelcount+=1
    lines.append(line)
  if len(labels)>0:
    ml=max(map(len,labels.values()))
  else:
    ml=-1
  newlines=[]
  for addr,opcode,*params in lines:
    #print((addr,opcode,*params))
    line=''
    if ml!=-1:
      if addr in labels:
        prefix=labels[addr]+':'
      else:
        prefix=''
      prefix=prefix.ljust(ml+2)
      line+=prefix
    line+=hexlen(addr,8)
    line+=' '
    line+=opcode.ljust(8)
    for ptype,param,size in params:
      line+=' '
      if ptype=='addr':
        line+=labels[param].ljust(8)
      elif ptype=='addroff':
        jaddr=addr+opsize(opcode)+param
        #print(labels,jaddr,addr,opcode,param)
        line+=labels[jaddr].ljust(8)
      else:
        line+=hexlen(param,size*2)
    newlines.append(line)
  if len(insts) in labels:
    line=labels[len(insts)]+':'
    newlines.append(line)
  #print(labels)
  return '\n'.join(newlines)