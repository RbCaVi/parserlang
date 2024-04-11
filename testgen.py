from gen import geneval,genfn
import sys
from stmt import expr,stmt
from disasm import disasm2,disasm
import itertools

try:
  batched=itertools.batched
except AttributeError:
  def batched(l,count):
    it=iter(l)
    while True:
      line=()
      for i in range(count):
        try:
          line=line+(next(it),)
        except StopIteration:
          break
      else:
        yield line
        continue
      yield line
      break

def parseexpr(s):
  return next(iter(expr(s)))[0]

def parsestmt(s):
  return next(iter(stmt(s)))[0]

def hexlen(n,l):
  return hex(n)[2:].rjust(l,'0')

def hexformat(s):
  step=8
  lines=[]
  for i,parts in enumerate(batched(s,step)):
    line=hexlen(i*step,8)
    for n in parts:
      line+=' '
      line+=hexlen(n,2)
    lines.append(line)
  return '\n'.join(lines)

def testexpr(code):
  print('code')
  print(code)
  insts=geneval(parseexpr(code),{})
  print('compiled')
  print(hexformat(insts))
  print('asm')
  #print(disasm(insts))
  #print('asm2')
  print(disasm2(insts))

def teststmt(code):
  print('code')
  print(code)
  insts=genfn(parsestmt(code),{})
  print('compiled')
  print(hexformat(insts))
  print('asm')
  #print(disasm(insts))
  #print('asm2')
  print(disasm2(insts))

testexpr('9')
testexpr('9+6')
#testexpr('+9')
testexpr('9?5:7?6:3')
#teststmt('9?5:7?6:3')
teststmt('fn a(a) a=1')
teststmt('fn a(a,b) a=1')
teststmt('fn a(a,b) b=1')
teststmt('fn a(a,b) def b=1')
teststmt('fn a(a,b){def b=1 a=b}')