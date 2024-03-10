from parser import printresults
from parsers import parserify
from evalexpr import evaluate

ep=parserify(evaluate)

printresults(ep,'')
printresults(ep,'1')
printresults(ep,'1+2')
printresults(ep,'(1)')
printresults(ep,'a(1)')
printresults(ep,'a(1,2)')
printresults(ep,'[]')
printresults(ep,'[1]')
printresults(ep,'[1,2]')