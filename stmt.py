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

@transform(concatstrip(strs('{'),star(noneerror(stmtwrap)),strs('}')))
def block(data):
	return ['STMT','many',*data[1]]

@parser
def sym(s):
	data,s=getSym(s)
	if data:
		yield data,s

typep=strip(alternate()) # no values

TYPE='TYPE'
STMT='STMT'
ARG='ARG'
SIG='SIG'
FN='FN'
anytype=[TYPE,'any']

def argp(data):
	typ,var=data
	if typ is None:
		typ = 'ANY'
	return [ARG,typ,var]

@transform(optional(concatstrip(concatstrip(optional(typep),optional(sym)),star(concatstrip(strs(','),concatstrip(optional(typep),optional(sym)))),optional(strs(',')))))
def commasep(data):
	if data is None:
		return []
	d,others,_=data
	return [argp(d),*[argp(arg) for _,arg in others]]

@transform(
	alternate(
		concatstrip(
			strs('fn'),concatstrip(sym,strs('('),commasep,strs(')')),alternate(block,stmtwrap)
		),
		concatstrip(
			strs('if'),expr,strs('then'),alternate(block,stmtwrap)
		),
		concatstrip(
			strs('def'),optional(typep),errorafter(sym),strs('='),expr
		),
		concatstrip(
			expr,strs('='),expr
		),
		concatstrip(
			expr,strs(*[op+'=' for op in ops]),expr
		),
		expr,
		block
	)
)
def stmt(data):
	if data[0]==0: # func
		_,(name,_,args,_),bstmtdata=data[1]
		if bstmtdata[0]==0:
			(_,_,*stmts) = bstmtdata[1]
		else:
			stmts = [bstmtdata[1]]
		return [STMT,'func',name,[SIG,*args],*stmts]
	if data[0]==1: # ifstmt
		_,cond,__,st=data[1]
		if st[0]==0:
			(_,_,*stmts) = st[1]
		else:
			stmts = [st[1]]
		return [STMT,'if',cond,*stmts]
	if data[0]==2: # declare
		_,typ,var,__,e=data[1]
		if typ is None:
			typ = 'ANY'
		return [STMT,'def',typ,var,e]
	if data[0]==3: # setstmt
		e1,_,e2=data[1]
		return [STMT,'set',e1,e2]
	if data[0]==4: # setop
		e1,aop,e2=data[1]
		return [STMT,'setop',aop,e1,e2]
	if data[0]==5: # exprstmt
		return [STMT,'expr',data[1]]
	if data[0]==6: # block
		return [BLOCK,data[1]]