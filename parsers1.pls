fn parsestr(str) {
	fn parsestr(s, str) {
		if strleft(s, len(str)) == str then {
			yield ([str, strmid(s, len(str))])
		}
	}
	return bind(parsestr, str)
}

fn concatp(parsers) {
	fn concatp(s, parsers) {
		def l = []
		def x = print("aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa")
		for parser in parsers do {
			def x = print(parser)
			def x = parser(s)
			l = concat(l, [x[0]])
			s = x[1]
		}
		return l
	}
	return bind(concatp, parsers)
}

fn alternate(parsers) {
	fn alternate(s, parsers) {
		for parser in parsers do {
			for x in gcall(parser, s) do {
				yield x
			}
		}
	}
	return bind(alternate, parsers)
}

def b = print(16)

for a in gcall(alternate([parsestr("j"), parsestr("jj")]), "jj") do {
	def b = print(a)
}

def p = alternate([parsestr("j"), parsestr("jj")])

for a in gcall(concatp([p, p]), "jj") do {
	def b = print(a)
}
