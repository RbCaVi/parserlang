def tests = [["str", "str"], ["str", "stra"], ]

def out = []

for a in tests do {
	def s1 = a[0]
	def s2 = a[1]

	out = concat(out, [[s1, s2, (s1 == s2)]])
}

return out