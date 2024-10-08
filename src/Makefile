# for f in $(find|grep '\.d'); do
#   fstem=$(echo ${f}|sed 's_^\./__;s/\.d//') 
#   echo -n ${fstem}.o:
#   cat $f |tr '\\' ' '|tr '\n' ' '|grep -Eo ' \b[^ ]+'|sort|tr -s '\n' ' '
#   echo 
# done | sed -E 's/^([^.]+)\.o(.*)\1.c \1.h(.*)$/\1.o\2\3/;s/  / /'
# compilecmd := gcc -MD -Wall -Wextra -Wpedantic -Wconversion -Werror=return-type -Werror=implicit-function-declaration -I . -c
compilecmd := gcc -Og -g -gdwarf-4 -ggdb -Wall -Wextra -Wpedantic -Wconversion -Werror=return-type -Werror=implicit-function-declaration -I pv -I . -c

ifeq ("x","y")
#https://stackoverflow.com/a/4493447
%.o: %.c # ignore the "implicit" rule from make
%.o: %.c %.h
	$(compilecmd) -Wl,-rdynamic $< -o $@


endif

%.o: %.c
	$(compilecmd) $< -o $@

%: %.o
	gcc $^ -o $@

.PHONY: all clean

all: pv_main main

# core
pv/pv.o: pv/pv.h pv/pv_private.h
pv/pv_private.o: pv/pv_private.h pv/pv.h
pv/pv_install.o: pv/pv_install.h pv/pv_singletons.h pv/pv_number.h pv/pv_string.h pv/pv_array.h pv/pv_object.h pv/pv.h

# aux (the stuff you have to register your kind for)
pv/pv_to_string.o: pv/pv_to_string.h pv/pv.h
pv/pv_hash.o: pv/pv_hash.h pv/pv.h
pv/pv_equal.o: pv/pv_equal.h pv/pv.h

# kinds
pv/pv_number.o: pv/pv_number.h pv/pv_to_string.h pv/pv_equal.h pv/pv.h
pv/pv_singletons.o: pv/pv_singletons.h pv/pv_to_string.h pv/pv_equal.h pv/pv.h
pv/pv_string.o: pv/pv_string.h pv/pv_private.h pv/pv_to_string.h pv/pv_hash.h pv/pv_equal.h pv/pv.h
pv/pv_array.o: pv/pv_array.h pv/pv_private.h pv/pv_to_string.h pv/pv_equal.h pv/pv.h
pv/pv_object.o: pv/pv_object.h pv/pv_private.h pv/pv_to_string.h pv/pv_hash.h pv/pv_equal.h pv/pv.h

# actual "vm" code
pl/pl_ffi.o: pl/util_pl.h pv/pv.h
pl/pl_stack.o: pl/util_pl.h pv/pv.h
pl/pl_dump.o: pl/util_pl.h pv/pv.h

# the executables
main.o: pv/pv.h pl/pl_stack.h pl/util_pl.h
pv_main.o: pv/pv_singletons.h pv/pv_number.h pv/pv_string.h pv/pv_array.h pv/pv_object.h pv/pv_to_string.h pv/pv_install.h pv/pv.h

pv_main: pv/pv.o pv/pv_private.o pv/pv_install.o \
	pv/pv_singletons.o pv/pv_number.o \
	pv/pv_string.o pv/pv_array.o pv/pv_object.o \
	pv/pv_to_string.o pv/pv_equal.o pv/pv_hash.o

main: pv/pv.o pv/pv_private.o pv/pv_install.o \
	pv/pv_singletons.o pv/pv_number.o \
	pv/pv_string.o pv/pv_array.o pv/pv_object.o \
	pv/pv_to_string.o pv/pv_equal.o pv/pv_hash.o \
	pl/util_pl.o pl/pl_dump.o pl/pl_stack.o

clean:
	rm -f main pv_main
	rm -f *.o
	rm -f pl/*.o
	rm -f pv/*.o