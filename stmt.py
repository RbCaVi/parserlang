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
	return ['STMT','many',*data]

@transform(concatstrip(strs('{'),stmts,strs('}')))
def block(data):
	return data[1]

@transform(alternate(block,stmtwrap))
def blockstmt(data):
	if data[0]==0:
		return data[1]
	else:
		return ['STMT','many',data[1]]

@parser
def sym(s):
	data,s=getSym(s)
	if data:
		yield data,s

@transform(concatstrip(expr,strs(*[op+'=' for op in ops]),expr))
def assign(data):
	e1,aop,e2=data
	return [STMT,aop,e1,e2]

@transform(concatstrip(expr,strs('='),expr))
def setstmt(data):
	e1,_,e2=data
	return [STMT,'set',e1,e2]

typep=strip(alternate()) # no values

TYPE='TYPE'
STMT='STMT'
ARG='ARG'
SIG='SIG'
FN='FN'
anytype=[TYPE,'any']

@transform(concatstrip(strs('def'),optional(typep),errorafter(sym),strs('='),expr))
def declare(data):
	_,typ,var,__,e=data
	return [STMT,'def',typ,var,e]

@transform(concatstrip(strs('if'),expr,block))
def ifstmt(data):
	_,cond,__,st,___=data
	return [STMT,'if',cond,*st]

def commasep(p):
	@transform(optional(concatstrip(p,star(concatstrip(strs(','),p)),optional(strs(',')))))
	def commasep(data):
		if data is None:
			return []
		d,others,_=data
		return [d,*[arg for _,arg in others]]
	return commasep

@transform(concatstrip(optional(typep),optional(sym)))
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
	return [STMT,'func',name,[SIG,*args],*stmts]

@transform(expr)
def exprstmt(data):
	return [STMT,'expr',data]

@transform(alternate(func,ifstmt,declare,setstmt,assign,exprstmt,block))
def stmt(data):
	if data[0]==5: # block
		return [BLOCK,*block]
	return data[1]