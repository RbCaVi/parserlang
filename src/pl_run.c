#include "pv/pv_number.h"
#include "pv/pv_array.h"
#include "pv/pv_install.h"

#include "pl/pl_dump.h"
#include "pl/pl_stack.h"
#include "pl/pl_bytecode.h"
#include "pl/pl_exec.h"
#include "pl/pl_func.h"
#include "pl/pl_iter.h"

#include "plc/plc_exe.h"

#include <stdio.h>
#include <stdlib.h>

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
	if (argc < 2) {
		return 1;
	}
	pv_install();
	pl_func_install();
	pl_iter_install();

	file_data f = readfile(argv[1]);

	plc_exe e = pl_exe_parse(f.data);

	pv main = e.main;

	pl_bytecode bytecode = pl_func_get_bytecode(pv_copy(main));
	printf("\nmain bytecode:\n");
	pl_bytecode_dump(bytecode);
	printf("\n");

	pl_state *pl = malloc(sizeof(pl_state));
	pl->stack = pl_stack_new();
	pl->globals = e.globals;

	pv ret = pl_func_call(main, pl);
	printf("return value:\n");
	pl_dump_pv(ret);
	printf("\n");

	for (unsigned int i = 0; i < e.glen; i++) {
		printf("global %i:\n", i);
		pl_dump_pv(pv_copy(pl->globals[i]));
		printf("\n");
	}

	pl_stack_unref(pl->stack);

	for (unsigned int i = 0; i < e.glen; i++) {
		pv_free(pl->globals[i]);
	}
	free(pl->globals);

	free(pl);

	free(f.data);

	return 0;
}