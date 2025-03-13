fn f(a, b) {
	yield ([a, b])
	yield ([(a + 1), b])
	yield ([(a + 2), b])
}

def x = 17

def f2 = bind(f, x)

# but what if:
# def f2 = bind(f: @, 6, @)

#fn f2(a, c) {
#	return f(a, 6, c)
#}

fn collect(it) {
	def l = []
	for x in it do {
		l = concat(l, [x])
	}
	return l
}

return collect(gcall(f2, 3))