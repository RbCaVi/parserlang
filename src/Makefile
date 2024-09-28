# for f in $(find|grep '\.d'); do
#   fstem=$(echo ${f}|sed 's_^\./__;s/\.d//') 
#   echo -n ${fstem}.o:
#   cat $f |tr '\\' ' '|tr '\n' ' '|grep -Eo ' \b[^ ]+'|sort|tr -s '\n' ' '
#   echo 
# done | sed -E 's/^([^.]+)\.o(.*)\1.c \1.h(.*)$/\1.o\2\3/;s/  / /'
# compilecmd := gcc -MD -Wall -Wextra -Wpedantic -Wconversion -Werror=return-type -Werror=implicit-function-declaration -I . -c
compilecmd := gcc -Wall -Wextra -Wpedantic -Wconversion -Werror=return-type -Werror=implicit-function-declaration -I pv2 -c

ifeq ("x","y")
#https://stackoverflow.com/a/4493447
%.o: %.c # ignore the "implicit" rule from make
%.o: %.c %.h
	$(compilecmd) $< -o $@

pl/pl_ffi.o: pl/util_pl.h pv/pv.h
pl/pl_stack.o: pl/util_pl.h pv/pv.h
pl/pl_dump.o: pl/util_pl.h pv/pv.h

pv/pv.o: pv/pvp.h pv/pv_alloc.h pv/pv_unicode.h pv/util.h
pv/pv_unicode.o: pv/pv_utf8_tables.h
pv/pv_aux.o: pv/pv.h pv/pv_aux.c pv/pv_alloc.h pv/pv_private.h
	# override the default recipe because there is no pv_aux.h
	$(compilecmd) $< -o $@

# the types
pv/pv_array.o: pv/pv.h
pv/pv_constants.o: pv/pv.h
pv/pv_number.o: pv/pv.h
pv/pv_object.o: pv/pv.h
pv/pv_string.o: pv/pv.h

main.o: main.c pv/pv.h pl/pl_stack.h pl/util_pl.h
	$(compilecmd) $< -o $@

main: main.o pl/util_pl.o pl/pl_dump.o pv/pv.o pv/pv_alloc.o pv/util.o pv/pv_unicode.o pv/pv_array.o pv/pv_constants.o pv/pv_number.o pv/pv_object.o pv/pv_string.o pv/pv_invalid.o pv/pv_constants.o
	gcc $^ -o $@

pv/pv.o: pv/pvp.h pv/pv_alloc.h pv/pv_unicode.h pv/util.h 
pv/pv_alloc.o: pv/pv.h 
pv/pv_unicode.o: pv/pv_utf8_tables.h 
pv/pv_array.o: pv/pv.h pv/pvp.h pv/pv_alloc.h pv/pv_constants.h pv/pv_invalid.h pv/pv_number.h pv/pv_string.h 
pv/pv_constants.o: pv/pv.h pv/pvp.h 
pv/pv_number.o: pv/pv.h pv/pvp.h 
pv/pv_object.o: pv/pv.h pv/pvp.h pv/pv_alloc.h pv/pv_constants.h pv/pv_invalid.h 
pv/pv_string.o: pv/pv.h pv/pvp.h pv/pv_alloc.h pv/pv_array.h pv/pv_invalid.h pv/pv_number.h pv/pv_unicode.h pv/util.h 
pv/pv_invalid.o: pv/pv.h pv/pvp.h pv/pv_alloc.h pv/pv_constants.h 
pl/pl_dump.o: pv/pv.h pl/util_pl.h pv/pv_array.h pv/pv_number.h pv/pv_object.h pv/pv_string.h 
main.o: main.c pv/pv.h pl/pl_dump.h pl/pl_stack.h pv/pv_array.h pv/pv_constants.h pv/pv_number.h pv/pv_object.h pv/pv_string.h

clean:
	rm -f main
	rm -f *.{o,d}
	rm -f pl/*.{o,d}
	rm -f pv/*.{o,d}
endif

%.o: %.c
	$(compilecmd) $< -o $@

# core
pv2/pv.o: pv2/pv.h pv2/pv_private.h
pv2/pv_private.o: pv2/pv_private.h pv2/pv.h

# aux (the stuff you have to register your kind for)
pv2/pv_to_string.o: pv2/pv_to_string.h pv2/pv.h
pv2/pv_hash.o: pv2/pv_hash.h pv2/pv.h
pv2/pv_equal.o: pv2/pv_equal.h pv2/pv.h

# kinds
pv2/pv_number.o: pv2/pv_number.h pv2/pv_to_string.h pv2/pv.h
pv2/pv_singletons.o: pv2/pv_singletons.h pv2/pv_to_string.h pv2/pv.h
pv2/pv_string.o: pv2/pv_string.h pv2/pv_private.h pv2/pv_to_string.h pv2/pv_hash.h pv2/pv_equal.h pv2/pv.h
pv2/pv_array.o: pv2/pv_array.h pv2/pv_private.h pv2/pv_to_string.h pv2/pv.h
pv2/pv_object.o: pv2/pv_object.h pv2/pv_private.h pv2/pv_to_string.h pv2/pv_hash.h pv2/pv_equal.h pv2/pv.h

pv2_test: pv2_main.o pv2/pv_number.o pv2/pv_to_string.o pv2/pv.o pv2/pv_private.o pv2/pv_singletons.o pv2/pv_string.o pv2/pv_array.o pv2/pv_object.o pv2/pv_equal.o pv2/pv_hash.o
	gcc $^ -o $@
