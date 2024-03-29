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

def printresults(p,s,error=False,formatter=None):
	print(repr(s))
	try:
		for data,s2 in p(s):
			if error:
				print('fail')
				break
			if formatter is not None:
				data=formatter(data)
			print(data,repr(s2))
	except Exception as e:
		if error:
			print('success',e)
		else:
			import traceback
			traceback.print_exc()