fn f1() {
	for x in [3] do {
		yield 3
	}
}

fn f2() {
	def x = f1()
}

return f2()
