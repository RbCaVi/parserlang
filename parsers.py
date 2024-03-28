from parser import parser

def strs(*ss):
	@parser
	def strs(s):
		for p in ss:
			if s.startswith(p):
				yield p,s[len(p):]
	return strs

def strip(p):
	@parser
	def strip(s):
		yield from p(s.lstrip())
	return strip

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
	@transform(star)
	def star(data):
		if data is None:
			return []
		return [data[0],*data[1]]
	newparser=optional(concat(p,star))
	return star

# use as decorator
# @transform(concat(a,b))
def transform(p):
	def transform(f):
		@parser
		def transform(s):
			for data,s2 in p(s):
				yield f(data),s2
		return transform
	return transform

# use as decorator
# function returning data,s pair
def parserify(f):
	@parser
	def parserified(s):
		yield f(s)
	return parserified

def optional(p,default=None):
	@parser
	def optional(s):
		yield from p(s)
		yield default,s
	return optional

def errornone(p):
	@parser
	def errornone(s):
		it=iter(p(s))
		try:
			yield next(it) # make sure there's at least one result
		except StopIteration:
			raise Exception('no choices')
		yield from it
	return errornone

def errorafter(p):
	@parser
	def errorafter(s):
		yield from p(s)
		raise Exception('ran out of choices')
	return errorafter

def atomic(p): # return 0 or 1 results
	@parser
	def atomic(s):
		it=iter(p(s))
		yield next(it)
	return atomic

def noneerror(p):
	@parser
	def noneerror(s):
		try:
			for x in p(s):
				yield x
		except:
			pass
	return noneerror