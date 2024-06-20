compilecmd := gcc -Wall -Wextra -Wpedantic -Wconversion -Werror=return-type -I . -c


pl/pl_ffi.o: pl/pl_ffi.c pl/pl_ffi.h pl/util_pl.h pv/pv.h
	$(compilecmd) $< -o $@
pl/pl_stack.o: pl/pl_stack.c pl/pl_stack.h pl/util_pl.h pv/pv.h
	$(compilecmd) $< -o $@

pv/pv.o: pv/pv.c pv/pv.h pv/pv_alloc.h pv/pv_unicode.h pv/util.h
	$(compilecmd) $< -o $@
pv/pv_unicode.o: pv/pv_unicode.c pv/pv_unicode.h pv/pv_utf8_tables.h
	$(compilecmd) $< -o $@
pv/pv_aux.o: pv/pv_alloc.h pv/pv_private.h
	# override the default recipe because there is no pv_aux.h
	$(compilecmd) $< -o $@

main.o: main.c main.h pv/pv.h pl/pl_stack.h pl/util_pl.h
	$(compilecmd) $< -o $@

main: main.o
	gcc $^ -o $@