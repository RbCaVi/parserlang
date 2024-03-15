from parsers import parserify,concat,alternate,strs,strip,transform
from expr import evaluate,EXPR,ops

expr=parserify(evaluate)

@transform(concat(expr,strip(strs('=',*[op+'=' for op in ops])),expr))
def assign(data):
	e1,aop,e2=data
	return [EXPR,aop,e1,e2]

@transform(alternate(assign))
def stmt(data):
	return data[1]