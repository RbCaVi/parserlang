#include "pl_bytecode.h"
#include "plc_parsetree.h"
#include "plc_codegen.h"

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
	if (argc < 2) {
		return 1;
	}

	file_data f = readfile(argv[1]);

	stmt s = parse_stmt(f.data);

	plc_codegen_context *c = plc_codegen_context_new();

	pl_bytecode_builder *b = plc_codegen_stmt(c, &s);

	pl_bytecode code = pl_bytecode_from_builder(b);
	pl_bytecode_dump(code);
	pl_bytecode_free(code);

	free_stmt(s);

	free(f.data);

	return 0;
}