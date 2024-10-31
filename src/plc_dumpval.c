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

	pv val = PV_ARRAY(pv_double(15), pv_double(3));

	pl_exe *exe = pl_exe_new();

	pl_exe_add_global(exe, val);

	pl_bytecode_builder *b = pl_bytecode_new_builder();
	pl_bytecode_builder_add(b, PUSHGLOBAL, {0});
	pl_bytecode_builder_add(b, RET, {});
	pl_bytecode bytecode = pl_bytecode_from_builder(b);

	pv f = pl_func(bytecode);

	pl_exe_set_main(exe, f);

	pl_exe_dump_file(exe, argv[1]);
	pl_exe_free(exe);

	return 0;
}