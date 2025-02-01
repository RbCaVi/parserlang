fn f1() {
	yield 1
	yield 2
	yield 3
}

fn f2() {
	return ([f1(), f1()])
}

fn collect(it) {
	def l = []
	for x in it do {
		l = concat(l, [x])
	}
	return l
}

fn f3() {
	return collect(gcall(f2))
}

return f3()
