#include "pv.h"
#include "pv_array.h"
#include "pv_number.h"
#include "pl_func.h"
#include "pl_bytecode.h"
#include "plc_exe.h"

int main(int argc, char **argv) {
	if (argc < 2) {
		return 1;
	}

	pv_array_install();
	pv_number_install();
	pl_func_install();

	pl_bytecode_builder *b = pl_bytecode_new_builder();
	pl_bytecode_builder_add(b, PUSHGLOBAL, {0});
	pl_bytecode_builder_add(b, RET, {});
	pl_bytecode bytecode = pl_bytecode_from_builder(b);
	
	pv f = pl_func(bytecode);
	pv *globals = malloc(sizeof(pv));
	globals[0] = PV_ARRAY(pv_double(15), pv_double(3));
	
	plc_exe exe = {f, 1, globals};

	plc_exe_dump(exe, argv[1]);

	return 0;
}