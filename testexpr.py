from parser import printresults
from parsers import parserify
from evalexpr import evaluate,treeexpr

def treeexpr2(expr):
	return treeexpr(expr)+'\n\n'

ep=parserify(evaluate)

printresults(ep,'',formatter=treeexpr2) # invalid: empty

# valid
printresults(ep,'1',formatter=treeexpr2) # number
printresults(ep,'a',formatter=treeexpr2) # symbol
printresults(ep,'1+2',formatter=treeexpr2) # binary operator
printresults(ep,'-2',formatter=treeexpr2) # unary operator
printresults(ep,'(1)',formatter=treeexpr2) # parentheses
printresults(ep,'-(2)',formatter=treeexpr2) # unary operator + parentheses
printresults(ep,'a(1)',formatter=treeexpr2) # function call
printresults(ep,'a(1,2)',formatter=treeexpr2) # function call multiple arguments
printresults(ep,'-a(2)',formatter=treeexpr2) # function call unary priority
printresults(ep,'(a)(2)',formatter=treeexpr2) # function in parentheses call
printresults(ep,'a(1,)',formatter=treeexpr2) # function dangling comma
printresults(ep,'a(1,2,)',formatter=treeexpr2) # function dangling comma
printresults(ep,'a[1]',formatter=treeexpr2) # index
printresults(ep,'a[1][2]',formatter=treeexpr2) # multiple index
printresults(ep,'(a)[2]',formatter=treeexpr2) # in parentheses index
printresults(ep,'[]',formatter=treeexpr2) # empty array
printresults(ep,'[1]',formatter=treeexpr2) # array
printresults(ep,'[1,2]',formatter=treeexpr2) # array one element
printresults(ep,'[1,]',formatter=treeexpr2) # array dangling comma
printresults(ep,'[1,2,]',formatter=treeexpr2) # array dangling comma
printresults(ep,'a.b',formatter=treeexpr2) # attribute
printresults(ep,'a.b.c',formatter=treeexpr2) # multiple attribute
printresults(ep,'a.b+1',formatter=treeexpr2) # attribute operator
printresults(ep,'1+a.b',formatter=treeexpr2) # operator attribute
printresults(ep,'-a.b',formatter=treeexpr2) # unary operator attribute
printresults(ep,'(a.b)',formatter=treeexpr2) # attribute in parentheses
printresults(ep,'(a).b',formatter=treeexpr2) # attribute + parentheses
printresults(ep,'a(b.c,d)',formatter=treeexpr2) # attribute before comma

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