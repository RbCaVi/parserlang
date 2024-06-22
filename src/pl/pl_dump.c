#include "pl/pl_dump.h"
#include "pl/util_pl.h"
#include "pv/pv_array.h"
#include "pv/pv_number.h"
#include "pv/pv_object.h"
#include "pv/pv_string.h"
#include "pl/pl_dump.h"

static void print_prefix(pl_dump_prefix parts) {
	printf(".");
	for (size_t i = 0; i < parts.count; i++) {
		pl_dump_prefix_part part = parts.data->parts[i];
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
	pl_dump_prefix parts;
	parts.data = checked_malloc(sizeof(size_t) + sizeof(pl_dump_prefix_part) * 4);
	parts.data->size = 4;
	parts.count = 0;
	pl_dump_pv_prefixed(val, parts);
}

void pl_dump_pv_prefixed(pv val, pl_dump_prefix parts) {
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
  	printf("\"%s\"\n",pv_string_value(val));
  	break;
  case PV_KIND_ARRAY:
  	printf("[]\n");
		inc_size(idx,parts.data,parts.count,sizeof(size_t),parts.data->size,(size_t)((float)parts.data->size * 1.5f));
		pv_array_foreach(val, i, v) {
			parts.data->parts[idx].type = IDX;
			parts.data->parts[idx].idx = i;
			pl_dump_pv_prefixed(v, parts);
  	}
  	break;
  case PV_KIND_OBJECT:
  	printf("{}\n");
		inc_size(idx,parts.data,parts.count,sizeof(size_t),parts.data->size,(size_t)((float)parts.data->size * 1.5f));
		pv_object_foreach(val, k, v) {
			parts.data->parts[idx].type = STR;
			parts.data->parts[idx].str = pv_string_value(k);
			pl_dump_pv_prefixed(v, parts);
  	}
  	break;
	}
}