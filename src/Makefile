# for f in $(find|grep '\.d'); do
#   fstem=$(echo ${f}|sed 's_^\./__;s/\.d//') 
#   echo -n ${fstem}.o:
#   cat $f |tr '\\' ' '|tr '\n' ' '|grep -Eo ' \b[^ ]+'|sort|tr -s '\n' ' '
#   echo 
# done | sed -E 's/^([^.]+)\.o(.*)\1.c \1.h(.*)$/\1.o\2\3/;s/  / /'
# compilecmd := gcc -MD -Wall -Wextra -Wpedantic -Wconversion -Werror=return-type -Werror=implicit-function-declaration -I . -c
compilecmd := gcc \
-Og -g -gdwarf-4 -ggdb \
-Wall -Wextra -Wpedantic -Wno-pedantic -Wno-comment \
-Wconversion -Werror=return-type -Werror=implicit-function-declaration \
-I pv -I pl -I plc -I . -c -MMD

colon := :
$(colon) := :

%.o: %.c
	$(compilecmd) -fPIC $< -o $@

%: %.o
	gcc -flto $^ -o $@ -lm -ldl -L. $(filter -l:%,$|)

%.so:
	gcc -shared -flto $^ -o $@ -L. $(filter -l:%,$|)

# because pattern rules can't be phony
FORCE:

test-%: FORCE %
	LD_LIBRARY_PATH="$$LD_LIBRARY_PATH":. $(*D)/$(*F) $(wordlist 3,$(words $^),$^)

test-valgrind-%: FORCE %
	LD_LIBRARY_PATH="$$LD_LIBRARY_PATH":. valgrind $(*D)/$(*F) $(wordlist 3,$(words $^),$^)

.PHONY: all clean

# this is a "hack" so i don't have to manage a variable for dependencies of each exe
.PHONY: -l\:pv.so -l\:pl.so

exes = pv_main pl_main pl_run plc_dumptree plc_dumpval plc_compile

all: $(exes) pt_dump.so
test: $(addprefix test-,$(exes))
test-valgrind: $(addprefix test-valgrind-,$(exes))

# core
pv/pv.o: pv/pv.h pv/pv_private.h
pv/pv_private.o: pv/pv_private.h
pv/pv_install.o: pv/pv_install.h \
	pv/pv_singletons.h pv/pv_number.h pv/pv_string.h pv/pv_array.h pv/pv_object.h pv/pv_userdata.h \
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
pv/pv_userdata.o: pv/pv_userdata.h \
	pv/pv_to_string.h pv/pv_equal.h \
	pv/pv.h
pl/pl_func.o: pl/pl_func.h \
	pl/pl_exec.h pl/pl_stack.h pl/pl_dump.h \
	pl/pl_bytecode.h pl/pl_opcodes.h pl/pl_opcodes_data.h pv/pv_number_ops_data.h \
	pv/pv.h pv/pv_private.h
pl/pl_iter.o: pl/pl_iter.h \
	pv/pv_number.h pv/pv_array.h pv/pv_object.h \
	pv/pv.h pv/pv_private.h \
	pl/pl_exec.h \
	pl/pl_stack.h pl/pl_dump.h \
	pl/pl_bytecode.h pl/pl_opcodes.h pl/pl_opcodes_data.h pv/pv_number_ops_data.h

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
	pl/pl_bytecode.h pl/pl_opcodes.h pl/pl_opcodes_data.h pv/pv_number_ops_data.h \
	pv/pv_singletons.h pv/pv_number.h pv/pv_array.h pv/pv_object.h pl/pl_func.h pl/pl_iter.h \
	pv/pv.h
pl/pl_bytecode.o: pl/pl_bytecode.h \
	pl/pl_opcodes.h  pl/pl_opcodes_data.h pv/pv_number_ops_data.h

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
	pl/pl_bytecode.h pl/pl_opcodes.h pl/pl_opcodes_data.h pv/pv_number_ops_data.h
pl_run.o: \
	pv/pv_number.h pv/pv_array.h \
	pl/pl_dump.h pl/pl_stack.h pl/pl_exec.h \
	pl/pl_bytecode.h pl/pl_opcodes.h pl/pl_opcodes_data.h pv/pv_number_ops_data.h \
	pl/pl_func.h pl/pl_iter.h \
	pv/pv.h pv/pv_install.h \
	plc/plc_exe.h

plc/plc_parsetree.o: plc/plc_op_ids.h

plc/plc_exe.o: plc/plc_exe.h pv/pv.h pv/pv_number.h pv/pv_array.h pl/pl_func.h pl/pl_bytecode.h pl/pl_opcodes.h pl/pl_opcodes_data.h pv/pv_number_ops_data.h pl/pl_dump.h pl/pl_stack.h pl/pl_exec.h

plc/plc_codegen.o: plc/plc_codegen.h plc/plc_parsetree.h

plc_dumptree.o: plc/plc_parsetree.h

plc_dumpval.o: plc/plc_exe.h pv/pv.h pv/pv_number.h pv/pv_array.h pl/pl_func.h pl/pl_bytecode.h pl/pl_opcodes.h pl/pl_opcodes_data.h pv/pv_number_ops_data.h pl/pl_dump.h pl/pl_stack.h pl/pl_exec.h

plc_compile.o:

pt_dump.o: pv/pv_to_string.h pl/pl_dump.h pv/pv.h pl/pl_stack.h pl/pl_exec.h pl/pl_bytecode.h pl/pl_opcodes.h pl/pl_opcodes_data.h pv/pv_number_ops_data.h

pv_main: | -l\:pv.so pv.so

pl_main: | -l\:pv.so pv.so -l\:pl.so pl.so pt_dump.so

pl_run: plc/plc_exe.o | -l\:pv.so pv.so -l\:pl.so pl.so

plc_dumptree: plc/plc_parsetree.o

plc_dumpval: plc/plc_exe.o | -l\:pv.so -l\:pl.so

plc_compile: plc/plc_parsetree.o plc/plc_codegen.o plc/plc_exe.o | -l\:pv.so -l\:pl.so

pv.so: pv/pv.o pv/pv_private.o pv/pv_install.o \
	pv/pv_singletons.o pv/pv_number.o \
	pv/pv_string.o pv/pv_array.o pv/pv_object.o pv/pv_userdata.o \
	pv/pv_to_string.o pv/pv_equal.o pv/pv_hash.o

pl.so: pl/pl_bytecode.o pl/pl_stack.o pl/pl_exec.o \
	pl/pl_iter.o pl/pl_func.o \
	pl/util_pl.o pl/pl_dump.o \
	| -l\:pv.so

pt_dump.so: pt_dump.o | -l\:pl.so pl.so

test-pl_run: ../main.plc
test-plc_dumptree: ../fib.plp
test-plc_dumpval: dump.plc

test-valgrind-pl_run: ../main.plc
test-valgrind-plc_dumptree: ../fib.plp
test-valgrind-plc_dumpval: dump.plc

.PHONY: dump.plc # hack i guess - test-plc_dumpval creates dump.plc

clean:
	rm -f pv_main pl_main pl_run plc_dumptree plc_dumpval plc_compile pt_dump.so
	rm -f pv.so pl.so
	rm -f *.o
	rm -f *.d
	rm -f pl/*.o
	rm -f pv/*.o