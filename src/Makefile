compilecmd := gcc -Wall -Wextra -Wpedantic -Wconversion -Werror=return-type -I . -c

%.o: %.c # ignore the "implicit" rule from make
%.o: %.c %.h
	$(compilecmd) $< -o $@

pl/pl_ffi.o: pl/util_pl.h pv/pv.h
pl/pl_stack.o: pl/util_pl.h pv/pv.h

pv/pv.o: pv/pv_alloc.h pv/pv_unicode.h pv/util.h
pv/pv_unicode.o: pv/pv_utf8_tables.h
pv/pv_aux.o: pv/pv_alloc.h pv/pv_private.h
	# override the default recipe because there is no pv_aux.h
	$(compilecmd) $< -o $@

main.o: pv/pv.h pl/pl_stack.h pl/util_pl.h

main: main.o
	gcc $^ -o $@