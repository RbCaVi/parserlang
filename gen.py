from opcodes import inst,bopcodes,uopcodes,opsize

# when an invalid is detected, instant return

def genfn(code,upvars={}):
  # code is [STMT,name,sig,*code]
  name=code[1]
  sig=code[2]
  stmts=code[3:]
  insts=b''
  vars={}
  for type,arg in getargs(sig):
    vars[arg]=len(vars)
  for _,upval in sorted(map(reversed,upvars.items())):
    vars[upval]=len(vars) # upvalues shadow arguments
  for stmt in stmts:
    stype=stmttype(stmt)
    if stype=='def': # assuming this doesn't happen in the middle of an expression
      var=getdefvar(stmt)
      expr=getdefexpr(stmt)
      if expr is not None:
        insts+=geneval(expr,vars)
      else:
        insts+=genpushinvalid(var+' is invalid')
      vars[var]=len(vars) # var def can override previous, but use it in expr
    elif stype=='set':
      var=getsetvar(stmt)
      var,path=getvarpath(var)
      expr=getsetexpr(stmt)
      insts+=geneval(expr,vars)
      insts+=gensetpath(vars[var],path)
    elif stype=='while':
      insts+=genwhile(stmt)
    elif stype=='for':
      insts+=genfor(stmt)
    elif stype=='if':
      insts+=genif(stmt)
    elif stype=='fn':
      stmt=setfnname(stmt,name+'('+getfnname(stmt))
      insts+=genupvaluefn(stmt,vars)
  return insts

def setfnname(stmt,name):
  #stmt=copy.deepcopy(stmt)
  stmt[1]=name
  return stmt

def getfnname(stmt):
  return stmt[1]

# funcs are resolved at compile time and are virtually immutable
# setting a func only changes variables set to it afterwards
# funcs assigned to vars change and are called normally

def geneval(expr,vars):
  etype=exprtype(expr)
  op=expr[1]
  args=expr[2:]
  insts=b''
  if etype=='(':
    # order:
    # local
    # global
    # function table
    fn=op
    if fn in vars:
      insts+=inst('DUPN',vars[fn])
      for expr in args:
        insts+=geneval(expr,vars)
      insts+=inst('CALLN',len(args))
    elif fn in globals:
      insts+=inst('PUSHCONST',globals[fn])
      for expr in args:
        insts+=geneval(expr,vars)
      insts+=inst('CALLN',len(args))
    elif fn in functable:
      for expr in args:
        insts+=geneval(expr,vars)
      insts+=inst('CALLCONSTFN',functable[fn],len(args))
  elif etype=='NUM':
    insts+=inst('PUSHNUM',op)
  elif len(args)==1:
    insts+=geneval(args[0],vars)
    insts+=genuop(op)
  elif len(args)==2:
    insts+=geneval(args[0],vars)
    insts+=geneval(args[1],vars)
    insts+=genbop(op)
  else:
    # special cases
    if op=='if':
      # eval after
      insts+=geneval(args[0],vars) # condition
      case1=geneval(args[1],vars)
      case2=geneval(args[2],vars)
      insts+=inst('JUMPF',len(case1)+opsize('JUMPF'))
      insts+=case1
      insts+=inst('JUMP',len(case2))
      insts+=case2
    else:
      raise Exception('unrecognized op')
  return insts

def exprtype(expr):
  return expr[0]

def genbop(op):
  return inst(bopcodes[op])

def genuop(op):
  return inst(uopcodes[op])