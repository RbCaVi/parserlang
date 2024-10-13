#include "pv/pv_number.h"
#include "pv/pv_array.h"
#include "pv/pv_install.h"

#include "pl/pl_dump.h"
#include "pl/pl_stack.h"
#include "pl/pl_bytecode.h"
#include "pl/pl_exec.h"
#include "pl/pl_func.h"
#include "pl/pl_iter.h"

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

typedef struct {
	unsigned int maini;
	unsigned int glen;
	unsigned int vlen;
} file_header;

typedef struct {
	pv main;
	unsigned int glen;
	pv *globals;
} pl_exe;

pv getvar(unsigned int i, char *data, unsigned int *vars, pv *vals) {
	if (pv_get_kind(vals[i]) == 0) {
		unsigned int pos = vars[i]; // offset into data

		pv val;
		unsigned int type =*((unsigned int *)(data + pos));
		printf("type %i at %i / %i\n", type, i, pos);
		switch (type) {
		case 0: // ntype = 0
			val = pv_double(*((double *)(data + pos + sizeof(unsigned int))));
			break;
		case 1: // atype = 1
			unsigned int alen = *((unsigned int *)(data + pos + sizeof(unsigned int)));
			unsigned int *varis = (unsigned int *)(data + pos + sizeof(unsigned int) + sizeof(unsigned int));
			pv a = pv_array();
			for (unsigned int i = 0; i < alen; i++) {
				a = pv_array_append(a, getvar(varis[i], data, vars, vals));
			}
			val = a;
			break;
		case 2: // ftype = 2
			unsigned int flen = *((unsigned int *)(data + pos + sizeof(unsigned int)));
			char *bytecode = data + pos + sizeof(unsigned int) + sizeof(unsigned int);
			val = pl_func((pl_bytecode){bytecode, flen, 0});
			break;
		default:
			abort(); // death
		}

		pv_free(vals[i]); // "just in case"
		vals[i] = val;
	}

	return pv_copy(vals[i]);
}

pl_exe processfile(file_data f) {
	file_header h = *((file_header*)f.data);
	unsigned int *globalis = (unsigned int *)(f.data + sizeof(file_header));
	unsigned int *vars = globalis + h.glen;
	pv *globals = malloc(h.glen * sizeof(pv));
	pv *vals = malloc(h.vlen * sizeof(pv));
	for (unsigned int i = 0; i < h.vlen; i++) {
		vals[i] = pv_invalid();
	}
	for (unsigned int i = 0; i < h.glen; i++) {
		printf("getting global %i\n", i);
		globals[i] = getvar(globalis[i], f.data, vars, vals);
	}
	printf("getting main\n");
	pv main = getvar(h.maini, f.data, vars, vals);
	for (unsigned int i = 0; i < h.vlen; i++) {
		pv_free(vals[i]);
	}
	free(vals);
	return (pl_exe){main, h.glen, globals};
}

int main(int argc, char **argv) {
	if (argc < 2) {
		return 1;
	}
	pv_install();
	pl_func_install();
	pl_iter_install();

	file_data f = readfile(argv[1]);

	pl_exe e = processfile(f);

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