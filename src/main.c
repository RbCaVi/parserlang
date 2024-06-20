#include "pv/pv.h"
#include "pl/pl_stack.h"
#include "pl/util_pl.h"

struct pl_dump_part {
	enum {
		STR,IDX,KEY
	} type;
	union {
		const char *str; // STR and KEY
		int idx;
	};
};

typedef struct {
	struct {
		size_t size;
		struct pl_dump_part parts[];
	} *data;
	size_t count;
} pl_dump_parts;

static void dump_pv_inner(pv val, pl_dump_parts parts);

static void print_prefix(pl_dump_parts parts) {
	printf(".");
	for (size_t i = 0; i < parts.count; i++) {
		struct pl_dump_part part = parts.data->parts[i];
		printf("/");
		switch (part.type) {
		case STR:
			printf("\"%s\"",part.str);
			break;
		case IDX:
			printf("%i",part.idx);
			break;
		case KEY:
			printf("%s",part.str);
			break;
		}
	}
	printf(": ");
}

void pl_dump_pv(pv val) {
	pl_dump_parts parts;
	parts.data = checked_malloc(sizeof(size_t) + sizeof(struct pl_dump_part) * 4);
	parts.data->size = 4;
	parts.count = 0;
	dump_pv_inner(val, parts);
}

static void dump_pv_inner(pv val, pl_dump_parts parts) {
	print_prefix(parts);
	size_t idx;
	switch (pv_get_kind(val)) {
  case PV_KIND_INVALID:
  	printf("ERROR OBJECT\n");
  	break;
  case PV_KIND_NULL:
  	printf("NULL\n");
  	break;
  case PV_KIND_FALSE:
  	printf("FALSE\n");
  	break;
  case PV_KIND_TRUE:
  	printf("TRUE\n");
  	break;
  case PV_KIND_NUMBER:
  	printf("%f\n",pv_number_value(val));
  	break;
  case PV_KIND_STRING:
  	printf("%s\n",pv_string_value(val));
  	break;
  case PV_KIND_ARRAY:
  	printf("[]\n");
		inc_size(idx,parts.data,parts.count,sizeof(size_t),parts.data->size,(size_t)((float)parts.data->size * 1.5f));
		pv_array_foreach(val, i, v) {
			parts.data->parts[idx].type = IDX;
			parts.data->parts[idx].idx = i;
			dump_pv_inner(v, parts);
  	}
  	break;
  case PV_KIND_OBJECT:
  	printf("{}\n");
		inc_size(idx,parts.data,parts.count,sizeof(size_t),parts.data->size,(size_t)((float)parts.data->size * 1.5f));
		pv_object_foreach(val, k, v) {
			parts.data->parts[idx].type = STR;
			parts.data->parts[idx].str = pv_string_value(k);
			dump_pv_inner(v, parts);
  	}
  	break;
	}
}

int main(int argc, char** argv) {
	pv n = PV_ARRAY(pv_number(15),pv_false());
	pl_dump_pv(n);
	return 0;
}
