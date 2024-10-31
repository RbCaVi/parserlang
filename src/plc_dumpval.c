#include "pv.h"
#include "pv_array.h"
#include "pv_number.h"
#include "pl_func.h"
#include "pl_bytecode.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
	unsigned int maini;
	unsigned int glen;
	int globalsallocsize;
	unsigned int *globals;
	unsigned int vlen;
	int valsallocsize;
	unsigned int *vals;
	unsigned int valdatalen;
	int valdataallocsize;
	char *valdata;
} exedata;

exedata *newexe() {
	exedata *out = malloc(sizeof(exedata));
	*out = (exedata){
		0,
		0,
		8,
		malloc(sizeof(int) * 8),
		0,
		8,
		malloc(sizeof(int) * 8),
		0,
		64,
		malloc(64),
	};
	return out;
}

void realloc_if_needed(void **ptr, unsigned int *size, size_t requiredsize) {
	if (requiredsize > *size) {
		while (requiredsize > *size) {
			*size = *size * 2;
		}
		//printf("(reallocing to size %i)", *size);
		*ptr = realloc(*ptr, *size);
	}
}

typedef struct __attribute__((packed)) {
	int type;
	double val;
} double_data;

typedef struct {
	int type;
	unsigned int len;
	int elements[];
} array_data;

typedef struct {
	int type;
	unsigned int len;
	char data[];
} func_data;

#define addentry(type, name, extrasize) \
	realloc_if_needed((void**)(&(data->vals)), &data->valsallocsize, sizeof(int) * (data->vlen + 1)); \
	unsigned int i = data->vlen; \
	data->vals[data->vlen] = data->valdatalen; \
	data->vlen++; \
	realloc_if_needed((void**)(&(data->valdata)), &data->valdataallocsize, data->valdatalen + sizeof(type) + extrasize); \
	type *name = (type*)(data->valdata + data->valdatalen); \
	data->valdatalen += (unsigned int)sizeof(type) + extrasize;

unsigned int addval(exedata *data, pv val) {
	pv_kind kind = pv_get_kind(val);
	//printf("kind %i\n", kind);
	if (kind == double_kind) {
		//printf("d %i ", data->valdatalen);
		addentry(double_data, d, 0);
		//printf("%i\n", data->valdatalen);
		d->type = 0;
		d->val = pv_number_value(val);
		return i;
	}
	if (kind == func_kind) {
		pl_bytecode b = pl_func_get_bytecode(val);
		//printf("f %i ", data->valdatalen);
		//printf("+ %i + %i = ", sizeof(func_data), b.length);
		addentry(func_data, f, b.length);
		//printf("%i\n", data->valdatalen);
		f->type = 2;
		f->len = b.length;
		memcpy(f->data, b.bytecode, b.length);
		b.freeable = 1;
		pl_bytecode_free(b);
		return i;
	}
	if (kind == array_kind) {
		unsigned int len = pv_array_length(pv_copy(val));
		unsigned int *valis = malloc(sizeof(int) * len);
		pv_array_foreach(val, i, v) {
			valis[i] = addval(data, v);
		}
		pv_free(val);
		//printf("a %i ", data->valdatalen);
		addentry(array_data, a, (unsigned int)sizeof(int) * len);
		//printf("%i\n", data->valdatalen);
		a->type = 1;
		a->len = len;
		memcpy(a->elements, valis, sizeof(int) * len);
		free(valis);
		return i;
	}
	abort(); // death
}

unsigned int addglobal(exedata *data, unsigned int vi) {
	realloc_if_needed((void*)(&(data->globals)), &data->globalsallocsize, sizeof(int) * (data->glen + 1));
	unsigned int i = data->glen;
	data->globals[data->glen] = vi;
	data->glen++;
	return i;
}

void setmain(exedata *data, unsigned int mi) {
	data->maini = mi;
}

void dumpexe(exedata *exe, char *file) {
	FILE *fptr = fopen(file, "wb");
	if (!fptr) {
		abort(); // death
	}
	fwrite(&(exe->maini), sizeof(int), 1, fptr);
	fwrite(&(exe->glen), sizeof(int), 1, fptr);
	fwrite(&(exe->vlen), sizeof(int), 1, fptr);
	fwrite(exe->globals, sizeof(int), exe->glen, fptr);
	unsigned int *fixedvals = malloc(sizeof(int) * exe->vlen);
	for (unsigned int i = 0; i < exe->vlen; i++) {
		fixedvals[i] = exe->vals[i] + (unsigned int)sizeof(int) + (unsigned int)sizeof(int) + (unsigned int)sizeof(int) + (unsigned int)sizeof(int) * exe->glen + (unsigned int)sizeof(int) * exe->vlen;
	}
	fwrite(fixedvals, sizeof(int), exe->vlen, fptr);
	free(fixedvals);
	fwrite(exe->valdata, 1, exe->valdatalen, fptr);
	fclose(fptr);
}

void freeexe(exedata *exe) {
	free(exe->globals);
	free(exe->vals);
	free(exe->valdata);
	free(exe);
}

int main(int argc, char **argv) {
	if (argc < 2) {
		return 1;
	}

	pv_array_install();
	pv_number_install();
	pl_func_install();

	pv val = PV_ARRAY(pv_double(15), pv_double(3));

	exedata *exe = newexe();

	unsigned int i1 = addval(exe, val);
	addglobal(exe, i1);

	pl_bytecode_builder *b = pl_bytecode_new_builder();
	pl_bytecode_builder_add(b, PUSHGLOBAL, {0});
	pl_bytecode_builder_add(b, RET, {});
	pl_bytecode bytecode = pl_bytecode_from_builder(b);

	pv f = pl_func(bytecode);

	unsigned int i2 = addval(exe, f);
	setmain(exe, i2);

	dumpexe(exe, argv[1]);
	freeexe(exe);

	return 0;
}