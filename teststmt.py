from parser import printresults
from parsers import parserify
from expr import treeexpr
from stmt import stmt

def treeexpr2(expr):
	return treeexpr(expr)+'\n\n'

printresults(stmt,'1=2',formatter=treeexpr2)
printresults(stmt,'1+=2',formatter=treeexpr2) # won't work until backtracking