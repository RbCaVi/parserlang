fn fib(x) {
	if x == 0 then return 0
	if x == 1 then return 1
	def a = fib(x - 1)
	def b = fib(x - 2)
	return a + b
}
return fib(5)