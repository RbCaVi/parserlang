set -ex

rm -f *.wasm *.js
rm -f *.o
rm -f pl/*.o
rm -f plc/*.o
rm -f pv/*.o

FLAGS="-g -profiling -std=c2x -Wno-pointer-sign -I pv -I pl -I plc -I ."

emcc $FLAGS -c pv/pv.c -o pv/pv.o
emcc $FLAGS -c pv/pv_private.c -o pv/pv_private.o
emcc $FLAGS -c pv/pv_install.c -o pv/pv_install.o

emcc $FLAGS -c pl/util_pl.c -o pl/util_pl.o

emcc $FLAGS -c pv/pv_to_string.c -o pv/pv_to_string.o
emcc $FLAGS -c pv/pv_hash.c -o pv/pv_hash.o
emcc $FLAGS -c pv/pv_equal.c -o pv/pv_equal.o

emcc $FLAGS -c pv/pv_number.c -o pv/pv_number.o
emcc $FLAGS -c pv/pv_singletons.c -o pv/pv_singletons.o
emcc $FLAGS -c pv/pv_string.c -o pv/pv_string.o
emcc $FLAGS -c pv/pv_array.c -o pv/pv_array.o
emcc $FLAGS -c pv/pv_object.c -o pv/pv_object.o
emcc $FLAGS -c pv/pv_userdata.c -o pv/pv_userdata.o
emcc $FLAGS -c pl/pl_func.c -o pl/pl_func.o
emcc $FLAGS -c pl/pl_iter.c -o pl/pl_iter.o

emcc $FLAGS -c pl/pl_stack.c -o pl/pl_stack.o
emcc $FLAGS -c pl/pl_dump.c -o pl/pl_dump.o
emcc $FLAGS -c pl/pl_exec.c -o pl/pl_exec.o
emcc $FLAGS -c pl/pl_bytecode.c -o pl/pl_bytecode.o
emcc $FLAGS -c pl/pl_builtins.c -o pl/pl_builtins.o

emcc $FLAGS -c plc/plc_parsetree.c -o plc/plc_parsetree.o
emcc $FLAGS -c plc/plc_exe.c -o plc/plc_exe.o
emcc $FLAGS -c plc/plc_codegen.c -o plc/plc_codegen.o

emcc $FLAGS -c pl_exec_run.c -o pl_exec_run.o

emcc -g -profiling -sENVIRONMENT=web \
	pl_exec_run.o \
	pl/pl_builtins.o \
	plc/plc_parsetree.o \
	plc/plc_codegen.o \
	plc/plc_exe.o \
	pv/pv.o \
	pv/pv_private.o \
	pv/pv_install.o \
	pv/pv_singletons.o \
	pv/pv_number.o \
	pv/pv_string.o \
	pv/pv_array.o \
	pv/pv_object.o \
	pv/pv_userdata.o \
	pv/pv_to_string.o \
	pv/pv_equal.o \
	pv/pv_hash.o \
	pl/pl_bytecode.o \
	pl/pl_stack.o \
	pl/pl_exec.o \
	pl/pl_iter.o \
	pl/pl_func.o \
	pl/util_pl.o \
	pl/pl_dump.o \
	-o pl_exec_run.js -s NO_EXIT_RUNTIME=1 -s "EXPORTED_RUNTIME_METHODS=['ccall']"