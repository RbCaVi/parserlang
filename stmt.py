from parsers import parserify,concat,alternate,strs,strip,transform
from expr import evaluate,EXPR,ops

ep=parserify(evaluate)

@transform(concat(ep,strip(strs('=',*[op+'=' for op in ops])),ep))
def assign(data):
	e1,aop,e2=data
	return [EXPR,aop,e1,e2]

@transform(alternate(assign))
def stmt(data):
	return data[1]