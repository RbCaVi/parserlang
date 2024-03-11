from parser import parser

def strs(ss):
	@parser
	def strs(s):
		for p in ss:
			if s.startswith(p):
				yield p,s[len(p):]
	return strs

def alternate(*ps):
	@parser
	def alternate(s):
		for i,p in enumerate(ps):
			for data,s2 in p(s):
				yield (i,data),s2
	return alternate

def concat(*ps):
	if len(ps)==0:
		@parser
		def concatinner(s):
			yield (),s
		return concatinner
	p1=ps[0]
	p2=concat(*ps[1:])
	@parser
	def concatinner(s):
		for data1,s2 in p1(s):
			for data2,s3 in p2(s2):
				yield (data1,)+data2,s3
	return concatinner

# def concat(p1,p2):
# 	@parser
# 	def concat(s):
# 		for data1,s2 in p1(s):
# 			for data2,s3 in p2(s2):
# 				yield (data1,data2),s3
# 	return concat

def star(p):
	newparser=None
	@parser
	def star(s):
		return newparser(s)
	newparser=alternate(concat(star,p),true)
	return star

# use as decorator
# @transform(concat(a,b))
def transform(p):
	def transform(f):
		@parser
		def transform(s):
			for data,i in p(s):
				yield f(data),i
		return transform
	return transform

# use as decorator
# function returning data,s pair
def parserify(f):
	@parser
	def parserified(s):
		try:
			yield f(s)
		except Exception as e:
			import traceback
			traceback.print_exc()
	return parserified