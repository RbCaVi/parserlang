from parser import parser
from parsers import parserify,concat,alternate,strs,strip,transform,optional,errornone,errorafter,star,noneerror
from expr import evaluate,EXPR,ops,getSym,SYM

expr=parserify(evaluate)

def concatstrip(*ps):
	return concat(*map(strip,ps))

def stmtwrap(s): # "backreference"
	return stmt(s)

@transform(star(noneerror(stmtwrap)))
def stmts(data):
	return ["BLOCK",*data]

@transform(concatstrip(strs('{'),stmts,strs('}')))
def block(data):
	return data[1]

@transform(alternate(block,stmtwrap))
def blockstmt(data):
	if data[0]==0:
		return data[1]
	else:
		return ["BLOCK",data[1]]

@parser
def sym(s):
	data,s=getSym(s)
	if data:
		yield data,s

@transform(concatstrip(expr,strs(*[op+'=' for op in ops]),expr))
def setop(data):
	e1,aop,e2=data
	return ["SETOPSTMT",aop,e1,e2]

@transform(concatstrip(expr,strs('='),expr))
def setstmt(data):
	e1,_,e2=data
	return ["SETSTMT",e1,e2]

typep=strip(alternate()) # no values

@transform(optional(typep))
def opttype(data):
	if data is None:
		return anytype
	return data

TYPE='TYPE'
ARG='ARG'
SIG='SIG'
anytype='any'

@transform(concatstrip(strs('def'),opttype,errorafter(sym),strs('='),expr))
def declare(data):
	_,typ,var,__,e=data
	return ['DEF',typ,var,e]

@transform(concatstrip(strs('if'),expr,strs('then'),alternate(block,stmtwrap)))
def ifstmt(data):
	_,cond,__,st=data
	if st[0]==0:
		(_,_,*stmts) = st[1]
	else:
		stmts = [st[1]]
	return ['IF',cond,["BLOCK",*stmts]]

def commasep(p):
	@transform(optional(concatstrip(p,star(concatstrip(strs(','),p)),optional(strs(',')))))
	def commasep(data):
		if data is None:
			return []
		d,others,_=data
		return [d,*[arg for _,arg in others]]
	return commasep

@transform(concatstrip(opttype,optional(sym)))
def arg(data):
	typ,var=data
	return [ARG,typ,var]

@transform(concatstrip(sym,strs('('),commasep(arg),strs(')')))
def funcsig(data):
	name,_,args,_=data
	return [SIG,name,*args]

@transform(concatstrip(strs('fn'),funcsig,blockstmt))
def func(data):
	_,(_,name,*args),(_,_,*stmts)=data
	return ["DEFFUNC",name,[SIG,*args],["BLOCK",*stmts]]

def one(p):
	@parser
	def one(s):
		yield next(iter(p(s)))
	return one

@transform(concatstrip(one(alternate(strs('return'),strs(''))),expr))
def exprstmt(data):
	(_,typ),e=data
	if typ == '':
		return ["EXPRSTMT",e]
	if typ == 'return':
		return ["RETURN",e]

@transform(alternate(func,ifstmt,declare,setstmt,setop,exprstmt,block))
def stmt(data):
	if data[0]==6: # block
		return ["BLOCK",*block]
	return data[1]