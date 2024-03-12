# to add a binary operator:
# add it to the ops list
# add it to precedences

ops=['+','-','*','/','^']

uops=['-']

precedences={
  '+':2,
  '-':2,
  '*':3,
  '/':3,
  '^':4,
}

def parseif(s):
  e1,s=evaluate(s)
  ss=s.strip()
  if not ss.startswith('then'):
    raise Exception('no then')
  s=ss[4:]
  e2,s=evaluate(s)
  ss=s.strip()
  if not ss.startswith('else'):
    raise Exception('no else')
  s=ss[4:]
  e3,s=evaluate(s)
  ss=s.strip()
  if not ss.startswith('end'):
    raise Exception('no end')
  s=ss[3:]
  return [EXPR,'if',e1,e2,e3],s

def ternary(s):
  e1,s=evaluate(s)
  ss=s.strip()
  if not ss.startswith(':'):
    raise Exception('expected :')
  s=ss[1:]
  e2,s=evaluate(s)
  ss=s.strip()
  return [EXPR,'if',e1,e2],s

# each key is a function returning (result,restofstr)
# result is an expression
# the values are the keywords that trigger using that parser
keywords={
  'if':parseif
}

# opkeywords have the immediately preceding expression added as the first argument to the returned expression
opkeywords={
  '?':ternary
}

# here starts my code
# please know what you are doing

# token types
NUM='NUM'   # number
LPAR='LPAR' # left paren
RPAR='RPAR' # right paren
OP='OP'     # binary operator
UOP='UOP'   # unary operator
SYM='SYM'   # symbol (variable or function)
EXPR='EXPR' # expression (output of evaluate)
CALL='CALL' # left paren after function name
IDX='IDX'
COMMA='COMMA'
LBR='LBR'
RBR='RBR'
KW='KW'
OPKW='OPKW'
DOT='DOT'

# from cpython Tokenize.py
def group(*choices): return '(' + '|'.join(choices) + ')'
def maybe(*choices): return group(*choices) + '?'

Hexnumber = r'0[xX](?:_?[0-9a-fA-F])+'
Binnumber = r'0[bB](?:_?[01])+'
Octnumber = r'0[oO](?:_?[0-7])+'
Decnumber = r'(?:0(?:_?0)*|[1-9](?:_?[0-9])*)'
Intnumber = group(Hexnumber, Binnumber, Octnumber, Decnumber)
Exponent = r'[eE][-+]?[0-9](?:_?[0-9])*'
Pointfloat = group(r'[0-9](?:_?[0-9])*\.(?:[0-9](?:_?[0-9])*)?',
                   r'\.[0-9](?:_?[0-9])*') + maybe(Exponent)
Expfloat = r'[0-9](?:_?[0-9])*' + Exponent
Floatnumber = group(Pointfloat, Expfloat)
Number = group(Floatnumber, Intnumber)
# end from cpython Tokenize.py

import re

def getNum(s):
  # an actual number
  # not a variable
  m=re.match(Floatnumber,s)
  if m:
    return float(s[:m.end()]),s[m.end():]
  m=re.match(Intnumber,s)
  if m:
    return int(s[:m.end()]),s[m.end():]
  return None,s

def getSym(s):
  # a symbol
  m=re.match('[a-zA-Z][a-zA-Z0-9]*',s)
  if m:
    return s[:m.end()],s[m.end():]
  return None,s

def moreTokens(s):
  # are there more tokens?
  return s.strip()!=''

def parenmatch(p1,p2):
  if p1[0] in [LPAR,CALL]:
    return p2[0]==RPAR
  if p1[0] in [IDX,LBR]:
    return p2[0]==RBR
  return False

def getToken(s,lastType,comma):
  # get one token
  # types accepted depand on last token
  # for example, can't have op after lpar
  # (*a) is not allowed
  ss=s.lstrip()
  if lastType==DOT:
    sym,snew=getSym(ss)
    if sym is not None:
      return [SYM,sym],snew
    raise Exception(f'no token: {s} after {lastType}')
  if lastType in [NUM,SYM,RPAR,RBR]:
    for op in ops:
      if ss.startswith(op):
        return [OP,op],ss[len(op):]
    if comma and ss.startswith(','):
      return [COMMA,','],ss[1:]
    if ss.startswith(')'):
      return [RPAR],ss[1:]
    if ss.startswith(']'):
      return [RBR],ss[1:]
    if ss.startswith('('):
      return [CALL],ss[1:]
    if ss.startswith('['):
      return [IDX],ss[1:]
    if ss.startswith('.'):
      return [DOT],ss[1:]
    for kw in opkeywords:
      if ss.startswith(kw):
        return [OPKW,kw],ss[len(kw):]
    raise Exception(f'no token: {s} after {lastType}')
  if lastType in [LPAR,CALL,IDX,OP,UOP,LBR,COMMA]:
    if ss.startswith('('):
      return [LPAR],ss[1:]
    if ss.startswith('['):
      return [LBR],ss[1:]
    if lastType in [CALL,COMMA] and ss.startswith(')'):
      return [RPAR],ss[1:]
    if lastType in [LBR,COMMA] and ss.startswith(']'):
      return [RBR],ss[1:]
    for uop in uops:
      if ss.startswith(uop):
        return [UOP,uop],ss[len(uop):]
    num,snew=getNum(ss)
    if num is not None:
      return [NUM,num],snew
    for kw in keywords:
      if ss.startswith(kw):
        return [KW,kw],ss[len(kw):]
    sym,snew=getSym(ss)
    if sym is not None:
      return [SYM,sym],snew
    raise Exception(f'no token: {s} after {lastType}')
  raise Exception(f'unrecognized type: {lastType}')

def precedence(token):
  # get the precedence of a binary operator
  return precedences[token[1]]

def rightassoc(op):
  return False

def leftassoc(op):
  return True

def apply(op,v1,v2):
  return [EXPR,op[1],v1,v2]

def applyuop(op,v):
  return [EXPR,op[1],v]

def evaluate(expr):
  values=[]
  ops=[]
  parens=[]

  s=expr

  # basic shunting yard parser
  lastType=OP # a valid expression can always come after an operator
  while moreTokens(s): # parse all the tokens
    try:
      token,s=getToken(s,lastType,len(parens) and parens[-1][0] in [CALL,LBR])
    except Exception as e:
      raise e
    if token[0]==KW:
      value,s=keywords[token[1]](s)
      values.append(value)
    if token[0]==OPKW:
      value,s=opkeywords[token[1]](s)
      values[-1]=value[:2]+[values[-1]]+value[2:]
    if token[0] in [NUM,SYM]: # number or symbol token
      values.append(token)
    if token[0] in [NUM,SYM,EXPR]: # number, symbol, or expression
      while len(ops)>0 and ops[-1][0]==UOP: # apply all unary operators on the stack
        op=ops[-1]
        ops=ops[:-1]
        values[-1]=applyuop(op,values[-1])
    if token[0] in [LPAR,CALL,IDX,LBR]: # left paren
      ops.append(token)
      parens.append(token)
      if token[0]==CALL:
        # unapply all unary operators
        par=ops[-1]
        ops=ops[:-1]
        while len(values[-1])==3:
          _,op,val=values[-1]
          ops.append([UOP,op])
          values[-1]=val
        ops.append(par)
        values[-1]=[EXPR,'(',values[-1]]
      if token[0]==IDX:
        values[-1]=[EXPR,'[',values[-1]]
      if token[0]==LBR:
        values.append([EXPR,'[]'])
    if token[0]==UOP: # unary operator
      ops.append(token)
    if token[0] in [RPAR,RBR]: # right paren
      if len(parens)==0:
        raise Exception(f'unopened {token[0]}')
      if not parenmatch(parens[-1],token):
        raise Exception(f'unmatched {parens[-1][0]} {token[0]}')
      while ops[-1][0] not in [LPAR,CALL,IDX,LBR]: # finish the parenthesized expression
        op=ops[-1]
        if op[0]==DOT:
          op=[OP,'.']
        ops=ops[:-1]
        v1,v2=values[-2:]
        values=values[:-2]
        values.append(apply(op,v1,v2))
      if ops[-1][0] in [CALL,IDX,LBR]:
        if lastType not in [ops[-1][0],COMMA]:
          arg=values[-1]
          values=values[:-1]
          values[-1].append(arg)
      ops=ops[:-1] # pop the left paren as well
      parens=parens[:-1]
    if token[0]==OP:
      while len(ops)>0 and (ops[-1][0]==DOT or (ops[-1][0] not in [LPAR,CALL,IDX] and precedence(ops[-1])>=precedence(token))):
        # apply all operators to the left with a lower precedence
        op=ops[-1]
        if op[0]==DOT:
          op=[OP,'.']
        ops=ops[:-1]
        v1,v2=values[-2:]
        values=values[:-2]
        values.append(apply(op,v1,v2))
      ops.append(token) # push this operator
    if token[0]==DOT:
      ops.append(token) # push this operator
    if token[0]==COMMA:
      while ops[-1][0] not in [CALL,IDX,LBR]:
        # apply all operators to the left with a lower precedence
        op=ops[-1]
        if op[0]==DOT:
          op=[OP,'.']
        ops=ops[:-1]
        v1,v2=values[-2:]
        values=values[:-2]
        values.append(apply(op,v1,v2))
      arg=values[-1]
      values=values[:-1]
      values[-1].append(arg)
    lastType=token[0] # type of last token
  while len(ops)>0: # apply the rest of the operators
    if ops[-1][0]==UOP:
      op=ops[-1]
      ops=ops[:-1]
      values[-1]=applyuop(op,values[-1])
    elif ops[-1][0] in [OP,DOT]:
      op=ops[-1]
      if op[0]==DOT:
        op=[OP,'.']
      ops=ops[:-1]
      v1,v2=values[-2:]
      values=values[:-2]
      values.append(apply(op,v1,v2))
    else:
      raise Exception(f'unclosed {ops[-1][0]}')
  if len(values)>1: # each operator reduces the number of values by 1
    raise Exception('not enough operators')
  if len(values)==0: # how
    raise Exception('empty expression')
  return values[0],s

def stringifyexpr(e):
  # convert an expr from evaluate to a string, so evaluate on that string will probably give back the same expr
  if e[0] in [SYM,NUM]:
    return str(e[1])
  if e[1]=='(':
    return f'{e[2]}({",".join(map(stringifyexpr,e[3:]))})'
  if len(e)==3:
    if len(e[2])==4:
      return f'{e[1]}({stringifyexpr(e[2])})' # -(a+b)
    return f'{e[1]}{stringifyexpr(e[2])}' # a
  if len(e)==4:
    _,op,left,right=e
    p1=precedence(e)
    # p1+1 because i don't want math.inf
    pleft=precedence(left) if left[0] in [OP,EXPR] else p1+1
    pright=precedence(right) if right[0] in [OP,EXPR] else p1+1
    leftparen=pleft<p1 or (pleft==p1 and rightassoc(op))
    rightparen=pright<p1 or (pright==p1 and leftassoc(op))
    sleft=stringifyexpr(left)
    if leftparen:
      sleft=f'({sleft})'
    sright=stringifyexpr(right)
    if rightparen:
      sright=f'({sright})'
    return f'{sleft}{op}{sright}'

# https://stackoverflow.com/a/76691030 (at least the box drawing characters)
def treeexpr(expr):
    elbow = "└─"
    pipe = "│ "
    tee = "├─"
    blank = "  "
    s=expr[0]+' '+str(expr[1])
    if expr[0]==EXPR:
      for i,e in enumerate(expr[2:]):
        top,*rest=treeexpr(e).split('\n')
        top=(elbow if i == len(expr) - 3 else tee)+top
        rest=[(blank if i == len(expr) - 3 else pipe)+line for line in rest]
        s='\n'.join([s,top,*rest])
    return s