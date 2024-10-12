#include "pv/pv_install.h"

#include "pl/pl_dump.h"
#include "pl/pl_stack.h"
#include "pl/pl_bytecode.h"
#include "pl/pl_exec.h"
#include "pl/pl_func.h"
#include "pl/pl_iter.h"

#include <stdio.h>
#include <stdlib.h>

// A function that will read a file at a path into an allocated char pointer buffer 
pl_bytecode readbytecode(char *path) {
	FILE *fptr = fopen(path, "rb"); // Open file for reading
	if (!fptr) {
		abort(); // death
	}
	fseek(fptr, 0, SEEK_END); // Seek to the end of the file
	unsigned int length = ftell(fptr); // Find out how many bytes into the file we are
	char *buf = (char*)malloc(length); // Allocate a buffer for the length of the file
	fseek(fptr, 0, SEEK_SET); // Go back to the beginning of the file
	fread(buf, length, 1, fptr); // Read the contents of the file into the buffer
	fclose(fptr); // Close the file

	return (pl_bytecode){buf, length}; // Return the buffer
}

int main(int argc, char **argv) {
	if (argc < 2) {
		return 1;
	}
	pv_install();
	pl_func_install();
	pl_iter_install();

	pl_bytecode bytecode = readbytecode(argv[1]);

	pl_bytecode_dump(bytecode);

	pv f = pl_func(bytecode);

	pl_state *pl = malloc(sizeof(pl_state));
	pl->stack = pl_stack_new();
	pl->globals = malloc(0 * sizeof(pv));

	pv ret = pl_func_call(f, pl);
	pl_dump_pv(ret);

	pl_stack_unref(pl->stack);

	free(pl->globals);

	free(pl);

	return 0;
}