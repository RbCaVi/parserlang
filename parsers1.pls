fn parsestr(str) {
	fn parsestr(s, str) {
		if strleft(s, len(str)) == str then {
			yield ([str, strmid(s, len(str))])
		}
	}
	return bind(parsestr, str)
}

fn concat(parsers) {
	fn concat(s, parsers) {
		def l = []
		for parser in parsers do {
			def x = parser(s)
			l = concat(l, [x[0]])
			s = x[1]
		}
		return l
	}
	return bind(concat, parsers)
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
