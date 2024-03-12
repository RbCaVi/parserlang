from parser import printresults
from parsers import parserify
from evalexpr import evaluate,treeexpr

def treeexpr2(expr):
	return treeexpr(expr)+'\n\n'

ep=parserify(evaluate)

printresults(ep,'',formatter=treeexpr2) # invalid: empty

# valid
printresults(ep,'1',formatter=treeexpr2)
printresults(ep,'1+2',formatter=treeexpr2)
printresults(ep,'-2',formatter=treeexpr2)
printresults(ep,'-(2)',formatter=treeexpr2)
printresults(ep,'-a(2)',formatter=treeexpr2)
printresults(ep,'(a)(2)',formatter=treeexpr2)
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
printresults(ep,'a.b',formatter=treeexpr2)
printresults(ep,'a.b.c',formatter=treeexpr2)
printresults(ep,'a.b+1',formatter=treeexpr2)
printresults(ep,'1+a.b',formatter=treeexpr2)
printresults(ep,'(a.b)',formatter=treeexpr2)
printresults(ep,'a(b.c,d)',formatter=treeexpr2)

# invalid: unmatched brackets
printresults(ep,'(1',error=True,formatter=treeexpr2)
printresults(ep,'1)',error=True,formatter=treeexpr2)
printresults(ep,'[1',error=True,formatter=treeexpr2)
printresults(ep,'1]',error=True,formatter=treeexpr2)
printresults(ep,'((1)',error=True,formatter=treeexpr2)
printresults(ep,'(1))',error=True,formatter=treeexpr2)
printresults(ep,'[1)',error=True,formatter=treeexpr2)
printresults(ep,'(1]',error=True,formatter=treeexpr2)
printresults(ep,'a(1]',error=True,formatter=treeexpr2)
printresults(ep,'a[1)',error=True,formatter=treeexpr2)
printresults(ep,'a(1',error=True,formatter=treeexpr2)
printresults(ep,'a[1',error=True,formatter=treeexpr2)

# invalid: dangling operator

# valid: custom syntax