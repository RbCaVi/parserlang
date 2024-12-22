#include "pv_install.h"
#include "pv_array.h"
#include "pl_bytecode.h"
#include "pl_func.h"
#include "pl_dump.h"
#include "plc_parsetree.h"
#include "plc_codegen.h"
#include "plc_exe.h"

#include <stdlib.h>
#include <stdio.h>

typedef struct {
	char *data;
	unsigned int length;
} file_data;

// A function that will read a file at a path into an allocated char pointer buffer 
file_data readfile(char *path) {
	FILE *fptr = fopen(path, "rb"); // Open file for reading
	if (!fptr) {
		abort(); // death
	}
	fseek(fptr, 0, SEEK_END); // Seek to the end of the file
	unsigned int length = (unsigned int)ftell(fptr); // Find out how many bytes into the file we are
	char *buf = (char*)malloc(length); // Allocate a buffer for the length of the file
	fseek(fptr, 0, SEEK_SET); // Go back to the beginning of the file
	fread(buf, length, 1, fptr); // Read the contents of the file into the buffer
	fclose(fptr); // Close the file

	return (file_data){buf, length}; // Return the buffer
}

int main(int argc, char **argv) {
	if (argc < 3) {
		return 1;
	}

	pv_install();

	pl_func_install();

	file_data f = readfile(argv[1]);

	stmt s = parse_stmt(f.data);

	plc_codegen_context *c = plc_codegen_context_new();

	pl_bytecode_builder *b = plc_codegen_stmt(c, &s);
	// in case control flow reaches the end, return null
	b = pl_bytecode_dup_builder(b); // the original is freed by plc_codegen_context_free(2
	pl_bytecode_builder_add(b, PUSHNULL, {});
	pl_bytecode_builder_add(b, RET, {});
	pl_bytecode_builder_add(b, GRET, {});
	pl_bytecode_builder_free(b);
	pl_bytecode code = pl_bytecode_from_builder(b);

	printf("top level bytecode:\n");
	pl_bytecode_dump(code);

	pv globals = plc_codegen_context_get_globals(c);

	plc_exe exe;

	exe.main = pl_func(code);

	exe.glen = pv_array_length(pv_copy(globals));
	exe.globals = malloc(exe.glen * sizeof(pv));

	pv_array_foreach(globals, i, v) {
		if (pv_get_kind(v) == func_kind) {
			printf("function at %i bytecode:\n", i);
			pl_bytecode_dump(pl_func_get_bytecode(pv_copy(v)));
		}
		exe.globals[i] = pv_copy(v);
	}

	pv_free(globals);

	plc_exe_dump(exe, argv[2]);

	for (unsigned int i = 0; i < exe.glen; i++) {
		pv_free(exe.globals[i]);
	}
	free(exe.globals);

	pl_bytecode_free(code);

	plc_codegen_context_free(c);

	free_stmt(s);

	free(f.data);

	return 0;
}