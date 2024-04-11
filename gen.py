from opcodes import inst,bopcodes,uopcodes,opsize

# when an invalid is detected, instant return

def genfn(code,upvars={}):
  # code is [STMT,'func',name,sig,*code]
  name=code[2]
  sig=code[3]
  stmts=code[4:]
  insts=b''
  vars={}
  for typ,arg in getargs(sig):
    vars[arg]=len(vars)
  for _,upval in sorted(map(reversed,upvars.items())):
    vars[upval]=len(vars) # upvalues shadow arguments
  print('vars',vars)
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
      insts+=gensetpath(vars[var],path,geneval(expr,vars))
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

def stmttype(stmt):
  return stmt[1]

def getsetvar(stmt):
  return stmt[2]

def getsetexpr(stmt):
  return stmt[3]

def getdefvar(stmt):
  return stmt[3]

def getdefexpr(stmt):
  return stmt[4]

def getvarpath(var):
  if var[0]=='SYM':
    return var[1],[]
  elif var[0]=='EXPR':
    raise Exception('expression lvalues aren\'t supported yet')
    if var[1]=='[':
      var,path=getvarpath(var)
      return var,[]+path

def gensetpath(varidx,path,expr):
  insts=b''
  if len(path)>0:
    raise Exception('expression lvalues aren\'t supported yet')
  else:
    insts+=inst('PUSHARRAY')
  insts+=expr
  insts+=inst('SETPATH',varidx)
  return insts

def getargs(args):
  _,*args=args
  print(args)
  return [(typ,name) for _,typ,name in args]

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
  if etype=='NUM':
    insts+=inst('PUSHNUM',op)
  elif etype=='SYM':
    var=op
    insts+=inst('DUPN',vars[var])
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
    elif op=='(':
      # order:
      # local
      # global
      # function table
      fn=args[0]
      args=args[1:]
      print(vars)
      if evalable(fn,vars):
        insts+=geneval(fn,vars)
        for expr in args:
          insts+=geneval(expr,vars)
        insts+=inst('CALLN',len(args))
      elif fn[0]=='SYM' and fn in functable:
        for expr in args:
          insts+=geneval(expr,vars)
        insts+=inst('CALLCONSTFN',functable[fn],len(args))
      else:
        raise Exception(f'couldn\'t find {fn}')
    elif len(args)==1:
      insts+=geneval(args[0],vars)
      insts+=genuop(op)
    elif len(args)==2:
      insts+=geneval(args[0],vars)
      insts+=geneval(args[1],vars)
      insts+=genbop(op)
    else:
      raise Exception(f'unrecognized op {op} in {expr}')
  return insts

def evalable(expr,vars):
  etype=exprtype(expr)
  op=expr[1]
  args=expr[2:]
  if etype=='NUM':
    return True
  elif etype=='SYM':
    return op in vars
  else:
    # special cases
    if op=='if':
      # eval after
      return (
        evalable(args[0],vars) and
        evalable(args[1],vars) and
        evalable(args[2],vars)
      )
    elif op=='(':
      # order:
      # local
      # global
      # function table
      fn=args[0]
      args=args[1:]
      print(vars)
      if evalable(fn,vars):
        out=True
        out=out and evalable(fn,vars)
        for expr in args:
          out=out and evalable(expr,vars)
        return out
      elif fn[0]=='SYM' and fn in functable:
        for expr in args:
          out=out and evalable(expr,vars)
        return out
      else:
        return False
    elif len(args)==1:
      try:
        genuop(op)
        return evalable(args[0],vars)
      except:
        return False
    elif len(args)==2:
      try:
        genbop(op)
        return evalable(args[0],vars) and evalable(args[1],vars)
      except:
        return False
    else:
      raise Exception(f'unrecognized op {op} in {expr}')
  return insts

def exprtype(expr):
  return expr[0]

def genbop(op):
  return inst(bopcodes[op])

def genuop(op):
  return inst(uopcodes[op])