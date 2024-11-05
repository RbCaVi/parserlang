#include "plc_exe.h"

#include "pv_array.h"
#include "pv_number.h"
#include "pl_func.h"
#include "pl_bytecode.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct plcp_exe {
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
};

typedef struct plcp_exe plcp_exe;

static plcp_exe *plcp_exe_new() {
	plcp_exe *out = malloc(sizeof(plcp_exe));
	*out = (plcp_exe){
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

static void realloc_if_needed(void **ptr, unsigned int *size, size_t requiredsize) {
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

static unsigned int addval(plcp_exe *data, pv val) {
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

static unsigned int plcp_exe_add_global(plcp_exe *data, pv val) {
	unsigned int vi = addval(data, val);
	realloc_if_needed((void*)(&(data->globals)), &data->globalsallocsize, sizeof(int) * (data->glen + 1));
	unsigned int i = data->glen;
	data->globals[data->glen] = vi;
	data->glen++;
	return i;
}

static void plcp_exe_set_main(plcp_exe *data, pv val) {
	unsigned int mi = addval(data, val);
	data->maini = mi;
}

static void plcp_exe_dump_file(plcp_exe *exe, char *file) {
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

static void plcp_exe_free(plcp_exe *exe) {
	free(exe->globals);
	free(exe->vals);
	free(exe->valdata);
	free(exe);
}

static pv getvar(unsigned int i, char *data, unsigned int *vars, pv *vals) {
	if (pv_get_kind(vals[i]) == 0) {
		unsigned int pos = vars[i]; // offset into data

		pv val;
		unsigned int type =*((unsigned int *)(data + pos));
		//printf("type %i at %i / %i\n", type, i, pos);
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

typedef struct {
	unsigned int maini;
	unsigned int glen;
	unsigned int vlen;
} plcp_exe_header;

plc_exe pl_exe_parse(char *data) {
	plcp_exe_header h = *((plcp_exe_header*)data);
	unsigned int *globalis = (unsigned int *)(data + sizeof(plcp_exe_header));
	unsigned int *vars = globalis + h.glen;
	pv *globals = malloc(h.glen * sizeof(pv));
	pv *vals = malloc(h.vlen * sizeof(pv));
	for (unsigned int i = 0; i < h.vlen; i++) {
		vals[i] = pv_invalid();
	}
	for (unsigned int i = 0; i < h.glen; i++) {
		//printf("getting global %i\n", i);
		globals[i] = getvar(globalis[i], data, vars, vals);
	}
	//printf("getting main\n");
	pv main = getvar(h.maini, data, vars, vals);
	for (unsigned int i = 0; i < h.vlen; i++) {
		pv_free(vals[i]);
	}
	free(vals);
	return (plc_exe){main, h.glen, globals};
}

void plc_exe_dump(plc_exe exe, char *filename) {
	plcp_exe *exedata = plcp_exe_new();

	for (int i = 0; i < exe.glen; i++) {
		plcp_exe_add_global(exedata, exe.globals[i]);
	}
	plcp_exe_set_main(exedata, exe.main);

	plcp_exe_dump_file(exedata, filename);
	plcp_exe_free(exedata);
}