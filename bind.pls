fn f(a, b) {
	yield ([a, b])
	yield ([(a + 1), b])
	yield ([(a + 2), b])
}

def f2 = bind(f, 6)

fn collect(it) {
	def l = []
	for x in it do {
		l = concat(l, [x])
	}
	return l
}

return collect(gcall(f2, 3))