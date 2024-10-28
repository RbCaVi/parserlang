# for f in $(find|grep '\.d'); do
#   fstem=$(echo ${f}|sed 's_^\./__;s/\.d//') 
#   echo -n ${fstem}.o:
#   cat $f |tr '\\' ' '|tr '\n' ' '|grep -Eo ' \b[^ ]+'|sort|tr -s '\n' ' '
#   echo 
# done | sed -E 's/^([^.]+)\.o(.*)\1.c \1.h(.*)$/\1.o\2\3/;s/  / /'
# compilecmd := gcc -MD -Wall -Wextra -Wpedantic -Wconversion -Werror=return-type -Werror=implicit-function-declaration -I . -c
compilecmd := gcc \
-Og -g -gdwarf-4 -ggdb \
-Wall -Wextra -Wpedantic -Wno-pedantic \
-Wconversion -Werror=return-type -Werror=implicit-function-declaration \
-I pv -I pl -I plc -I . -c

%.o: %.c
	$(compilecmd) $< -o $@

%: %.o
	gcc $^ -o $@ -lm

.PHONY: all clean

all: pv_main pl_main

# core
pv/pv.o: pv/pv.h pv/pv_private.h
pv/pv_private.o: pv/pv_private.h
pv/pv_install.o: pv/pv_install.h \
	pv/pv_singletons.h pv/pv_number.h pv/pv_string.h pv/pv_array.h pv/pv_object.h \
	pv/pv.h

pl/util_pl.o: pl/util_pl.h

# aux (the stuff you have to register your kind for)
pv/pv_to_string.o: pv/pv_to_string.h pv/pv.h
pv/pv_hash.o: pv/pv_hash.h pv/pv.h
pv/pv_equal.o: pv/pv_equal.h pv/pv.h

# kinds
pv/pv_number.o: pv/pv_number.h \
	pv/pv_to_string.h pv/pv_equal.h \
	pv/pv.h
pv/pv_singletons.o: pv/pv_singletons.h \
	pv/pv_to_string.h pv/pv_equal.h \
	pv/pv.h
pv/pv_string.o: pv/pv_string.h \
	pv/pv_to_string.h pv/pv_hash.h pv/pv_equal.h \
	pv/pv.h pv/pv_private.h
pv/pv_array.o: pv/pv_array.h \
	pv/pv_to_string.h pv/pv_equal.h \
	pv/pv.h pv/pv_private.h
pv/pv_object.o: pv/pv_object.h \
	pv/pv_to_string.h pv/pv_hash.h pv/pv_equal.h \
	pv/pv.h pv/pv_private.h
pl/pl_func.o: pl/pl_func.h \
	pl/pl_exec.h pl/pl_stack.h pl/pl_dump.h \
	pl/pl_bytecode.h pl/pl_opcodes.h pl/pl_opcodes_data.h \
	pv/pv.h pv/pv_private.h
pl/pl_iter.o: pl/pl_iter.h \
	pv/pv_number.h pv/pv_array.h pv/pv_object.h \
	pv/pv.h pv/pv_private.h

# actual "vm" code
#pl/pl_ffi.o: pl/util_pl.h pv/pv.h
pl/pl_stack.o: pl/pl_stack.h \
	pl/pl_dump.h pl/util_pl.h \
	pv/pv.h
pl/pl_dump.o: pl/pl_dump.h \
	pv/pv_singletons.h pv/pv_number.h pv/pv_string.h pv/pv_array.h pv/pv_object.h \
	pl/util_pl.h pv/pv.h
pl/pl_exec.o: pl/pl_exec.h \
	pl/pl_stack.h pl/pl_dump.h \
	pl/pl_bytecode.h pl/pl_opcodes.h pl/pl_opcodes_data.h \
	pv/pv_singletons.h pv/pv_number.h pv/pv_array.h pv/pv_object.h pl/pl_func.h pl/pl_iter.h \
	pv/pv.h
pl/pl_bytecode.o: pl/pl_bytecode.h \
	pl/pl_opcodes.h  pl/pl_opcodes_data.h

# the executables
pv_main.o: \
	pv/pv_singletons.h pv/pv_number.h pv/pv_string.h pv/pv_array.h pv/pv_object.h \
	pv/pv_to_string.h \
	pv/pv.h pv/pv_install.h
pl_main.o: \
	pv/pv_singletons.h pv/pv_number.h pv/pv_string.h pv/pv_array.h pv/pv_object.h pl/pl_func.h pl/pl_iter.h \
	pv/pv_to_string.h \
	pv/pv.h pv/pv_install.h \
	pl/pl_dump.h pl/pl_stack.h pl/pl_exec.h \
	pl/pl_bytecode.h pl/pl_opcodes.h pl/pl_opcodes_data.h
pl_run.o: \
	pv/pv_number.h pv/pv_array.h \
	pl/pl_dump.h pl/pl_stack.h pl/pl_exec.h \
	pl/pl_bytecode.h pl/pl_opcodes.h pl/pl_opcodes_data.h \
	pl/pl_func.h pl/pl_iter.h \
	pv/pv.h pv/pv_install.h

plc_parsetree.o: plc/plc_op_ids.h

pv_main: pv/pv.o pv/pv_private.o pv/pv_install.o \
	pv/pv_singletons.o pv/pv_number.o \
	pv/pv_string.o pv/pv_array.o pv/pv_object.o \
	pv/pv_to_string.o pv/pv_equal.o pv/pv_hash.o

pl_main: pv/pv.o pv/pv_private.o pv/pv_install.o \
	pv/pv_singletons.o pv/pv_number.o \
	pv/pv_string.o pv/pv_array.o pv/pv_object.o \
	pv/pv_to_string.o pv/pv_equal.o pv/pv_hash.o \
	pl/util_pl.o pl/pl_dump.o pl/pl_stack.o \
	pl/pl_bytecode.o pl/pl_exec.o pl/pl_func.o pv/pv_to_string.o pl/pl_iter.o

pl_run: pl/pl_dump.o pl/pl_stack.o pv/pv_install.o pv/pv_equal.o pv/pv_object.o pv/pv_hash.o pv/pv_singletons.o pv/pv_to_string.o pl/pl_iter.o pv/pv_string.o pv/pv_array.o pv/pv_private.o pl/pl_bytecode.o pl/pl_exec.o pl/pl_func.o pv/pv.o pl/util_pl.o pv/pv_number.o

plc_parsetree:

clean:
	rm -f pv_main pl_main pl_run
	rm -f *.o
	rm -f pl/*.o
	rm -f pv/*.o