fn f1() {
	yield 1
	yield 2
	yield 3
}

fn iterx(l) {
	for x in l do {
		yield x
	}
}

fn collect(it) {
	def l = []
	for x in it do {
		l = concat(l, [x])
	}
	return l
}

fn f2() {
	def a = each(gcall(f1))
	def b = each(gcall(f1))
	def c = each(gcall(f1))
	return ([a, b, c])
}

fn f3() {
	def it = gcall(f2)
	return collect(it)
}

fn f4() {
	def it = gcall(iterx, [1, 2, 3])
	def l = []
	for x in it do {
		l = concat(l, [x])
	}
	return l
}

return ([f3(), f4()])