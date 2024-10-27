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
def setop(data):
	e1,aop,e2=data
	return [STMT,'setop',aop,e1,e2]

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

@transform(concatstrip(strs('if'),expr,strs('then'),alternate(block,stmtwrap)))
def ifstmt(data):
	_,cond,__,st=data
	if st[0]==0:
		(_,_,*stmts) = st[1]
	else:
		stmts = [st[1]]
	return [STMT,'if',cond,[STMT,'many',*stmts]]

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
	if typ is None:
		typ = 'ANY'
	return [ARG,typ,var]

@transform(concatstrip(sym,strs('('),commasep(arg),strs(')')))
def funcsig(data):
	name,_,args,_=data
	return [SIG,name,*args]

@transform(concatstrip(strs('fn'),funcsig,blockstmt))
def func(data):
	_,(_,name,*args),(_,_,*stmts)=data
	return [STMT,'func',name,[SIG,*args],[STMT,'many',*stmts]]

def one(p):
	@parser
	def one(s):
		yield next(iter(p(s)))
	return one

@transform(concatstrip(one(alternate(strs('return'),strs(''))),expr))
def exprstmt(data):
	(_,typ),e=data
	if typ == '':
		return [STMT,'expr',e]
	if typ == 'return':
		return [STMT,'return',e]

@transform(alternate(func,ifstmt,declare,setstmt,setop,exprstmt,block))
def stmt(data):
	if data[0]==6: # block
		return [BLOCK,*block]
	return data[1]