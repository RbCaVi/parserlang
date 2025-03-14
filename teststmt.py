from parser import printresults
from parsers import parserify
from expr import treeexpr
from stmt import stmt,stmts

def treeexpr2(expr):
	return treeexpr(expr)+'\n\n'

printresults(stmt,'1=[]',formatter=treeexpr2)
printresults(stmt,'return []',formatter=treeexpr2) # thinks this is an index???? (i thought it would try return first)
printresults(stmt,'return [1]',formatter=treeexpr2)
printresults(stmt,'return ([])',formatter=treeexpr2)
printresults(stmt,'1=2',formatter=treeexpr2)
printresults(stmt,'1+=2',formatter=treeexpr2) # won't work until backtracking
printresults(stmt,'def a=2',formatter=treeexpr2)
printresults(stmt,'def 1=2',error=True,formatter=treeexpr2)
printresults(stmt,'if a then b=c',formatter=treeexpr2)
printresults(stmt,'if a then b',formatter=treeexpr2)
printresults(stmt,'fn a(a)',formatter=treeexpr2)
printresults(stmt,'fn a(b) c=d',formatter=treeexpr2)
printresults(stmts,'fn a(b) c=d fn aef(aweda) r=w',formatter=treeexpr2)
