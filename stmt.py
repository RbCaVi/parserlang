from parser import parser
from parsers import parserify,concat,alternate,strs,strip,transform,optional,errornone,errorafter
from expr import evaluate,EXPR,ops,getSym,SYM

expr=parserify(evaluate)

def stmtwrap(s): # "backreference"
	return stmt(s)

def concatstrip(*ps):
	return concat(*map(strip,ps))

@parser
def sym(s):
	data,s=getSym(s)
	if data:
		yield [SYM,data],s

@transform(concatstrip(expr,strs('=',*[op+'=' for op in ops]),expr))
def assign(data):
	e1,aop,e2=data
	return [STMT,aop,e1,e2]

typep=strip(alternate()) # no values

TYPE='TYPE'
STMT='STMT'
anytype=[TYPE,'any']

@transform(concatstrip(strs('def'),optional(typep),errorafter(sym),strs('='),expr))
def declare(data):
	_,typ,var,__,e=data
	if not typ:
		typ=anytype
	return [STMT,'def',typ,var,e]

@transform(concatstrip(strs('if'),expr,strs('then'),stmtwrap,strs('end')))
def ifstmt(data):
	_,cond,__,s,___=data
	return [STMT,'if',typ,var,e]

@transform(alternate(assign,declare))
def stmt(data):
	return data[1]