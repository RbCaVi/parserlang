from parser import parser
from parsers import parserify,concat,alternate,strs,strip,transform,optional,errornone,errorafter
from expr import evaluate,EXPR,ops,getSym,SYM

expr=parserify(evaluate)

def concatstrip(*ps):
	return concat(*map(ps))

@parser
def sym(s):
	data,s=getSym(s)
	if data:
		yield [SYM,data],s

@transform(concatstrip(expr,strs('=',*[op+'=' for op in ops]),expr))
def assign(data):
	e1,aop,e2=data
	return [EXPR,aop,e1,e2]

typep=strip(alternate()) # no values

TYPE='TYPE'
anytype=[TYPE,'any']

@transform(concatstrip(strs('def'),optional(typep),errorafter(sym),strs('='),expr))
def declare(data):
	_,typ,var,__,e=data
	if not typ:
		typ=anytype
	return [EXPR,'def',typ,var,e]

@transform(alternate(assign,declare))
def stmt(data):
	return data[1]