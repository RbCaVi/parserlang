class ParserResult:
	def __init__(self,gen):
		self.results=gen
	def __iter__(self):
		for x in self.results:
			if isinstance(x,ParserResult):
				yield from x
			else:
				yield x

def parser(f):
	def f2(s,*args,**kwargs):
		return ParserResult(f(s,*args,**kwargs))
	return f2

def printresults(p):
	for data,s in p:
		print(data,repr(s))