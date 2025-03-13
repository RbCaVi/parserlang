fn parsestr(s) {
	yield (["j", strmid(s, len("j"))])
}

fn concatp(s) {
	def x = parsestr(s)
}

def b = print(16)

for a in gcall(concatp, "j") do {
	def b = print(a)
}
