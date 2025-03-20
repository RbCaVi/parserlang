%.o: %.c
	emcc -std=c2x -Wno-pointer-sign -I pv -I pl -I plc -I . -c $< -o $@

.PHONY: all clean

all: pl_exec_run.js

pv/pv.o:
pv/pv_private.o:
pv/pv_install.o:

pl/util_pl.o:

pv/pv_to_string.o:
pv/pv_hash.o:
pv/pv_equal.o:

pv/pv_number.o:
pv/pv_singletons.o:
pv/pv_string.o:
pv/pv_array.o:
pv/pv_object.o:
pv/pv_userdata.o:
pl/pl_func.o:
pl/pl_iter.o:

pl/pl_stack.o:
pl/pl_dump.o:
pl/pl_exec.o:
pl/pl_bytecode.o:
pl/pl_builtins.o:

plc/plc_parsetree.o:
plc/plc_exe.o:
plc/plc_codegen.o:

pl_exec_run.o:

pl_exec_run.js: pl_exec_run.o pl/pl_builtins.o plc/plc_parsetree.o plc/plc_codegen.o plc/plc_exe.o pv/pv.o pv/pv_private.o pv/pv_install.o 	pv/pv_singletons.o pv/pv_number.o 	pv/pv_string.o pv/pv_array.o pv/pv_object.o pv/pv_userdata.o 	pv/pv_to_string.o pv/pv_equal.o pv/pv_hash.o pl/pl_bytecode.o pl/pl_stack.o pl/pl_exec.o 	pl/pl_iter.o pl/pl_func.o pl/util_pl.o pl/pl_dump.o
	emcc -sENVIRONMENT=web $^ -o $@ -s NO_EXIT_RUNTIME=1 -s "EXPORTED_RUNTIME_METHODS=['ccall']"

clean:
	rm -f *.wasm *.js
	rm -f *.o
	rm -f pl/*.o
	rm -f plc/*.o
	rm -f pv/*.o