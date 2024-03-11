from parser import printresults
from parsers import parserify
from evalexpr import evaluate,treeexpr

def treeexpr2(expr):
	return '\n\n'+treeexpr(expr)+'\n'

ep=parserify(evaluate)

printresults(ep,'',formatter=treeexpr2)
printresults(ep,'1',formatter=treeexpr2)
printresults(ep,'1+2',formatter=treeexpr2)
printresults(ep,'(1)',formatter=treeexpr2)
printresults(ep,'a[1]',formatter=treeexpr2)
printresults(ep,'a[1][2]',formatter=treeexpr2)
printresults(ep,'(a)[2]',formatter=treeexpr2)
printresults(ep,'a(1)',formatter=treeexpr2)
printresults(ep,'a(1,2)',formatter=treeexpr2)
printresults(ep,'[]',formatter=treeexpr2)
printresults(ep,'[1]',formatter=treeexpr2)
printresults(ep,'[1,2]',formatter=treeexpr2)
printresults(ep,'a(1,)',formatter=treeexpr2)
printresults(ep,'a(1,2,)',formatter=treeexpr2)
printresults(ep,'[1,]',formatter=treeexpr2)
printresults(ep,'[1,2,]',formatter=treeexpr2)