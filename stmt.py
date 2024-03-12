from parsers import parserify,concat,alternate,strp,strip,transform
from expr import evaluate,EXPR

ep=parserify(evaluate)

@transform(concat(ep,strip(strp('=')),ep))
def assign(data):
	e1,_,e2=data
	return [EXPR,'=',e1,e2]

@transform(alternate(assign))
def stmt(data):
	return data[1]