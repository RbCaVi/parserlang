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

SIG='SIG'

@transform(concatstrip(strs('def'),errorafter(sym),strs('='),expr))
def declare(data):
	_,var,__,e=data
	return ['DEF',var,e]

@transform(concatstrip(strs('if'),expr,strs('then'),blockstmt))
def ifstmt(data):
	_,cond,__,(_,*stmts)=data
	return ['IF',cond,["BLOCK",*stmts]]

def commasep(p):
	@transform(optional(concatstrip(p,star(concatstrip(strs(','),p)),optional(strs(',')))))
	def commasep(data):
		if data is None:
			return []
		d,others,_=data
		return [d,*[arg for _,arg in others]]
	return commasep

arg = sym

@transform(concatstrip(sym,strs('('),commasep(arg),strs(')')))
def funcsig(data):
	name,_,args,_=data
	return [SIG,name,*args]

@transform(concatstrip(strs('fn'),funcsig,blockstmt))
def func(data):
	_,(_,name,*args),(_,*stmts)=data
	return ["DEFFUNC",name,[SIG,*args],["BLOCK",*stmts]]

def one(p):
	@parser
	def one(s):
		yield next(iter(p(s)))
	return one

@transform(concatstrip(one(alternate(strs('return'),strs('yield'),strs(''))),expr))
def exprstmt(data):
	(_,typ),e=data
	if typ == '':
		return ["EXPRSTMT",e]
	if typ == 'return':
		return ["RETURNV",e]
	if typ == 'yield':
		return ["YIELD",e]

@transform(concatstrip(strs('for'),sym,strs('in'),expr,strs('do'),blockstmt))
def forstmt(data):
	_,var,__,it,___,(_,*stmts)=data
	return ['FOR',var,it,["BLOCK",*stmts]]

@transform(concatstrip(strs('while'),expr,strs('do'),blockstmt))
def whilestmt(data):
	_,cond,__,(_,*stmts)=data
	return ['WHILE',cond,["BLOCK",*stmts]]

@transform(concatstrip(strs('return')))
def returnstmt(data):
	return ['RETURN']

@transform(alternate(func,ifstmt,forstmt,whilestmt,declare,setstmt,setop,block,exprstmt,returnstmt))
def stmt(data):
	return data[1]