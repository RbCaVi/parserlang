fn f1() {
	def l = []
	yield 3
}

fn f2() {
	def x = f1()
}

return f2()
