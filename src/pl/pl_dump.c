#include "pl/pl_dump.h"
#include "pl/util_pl.h"
#include "pv/pv_singletons.h"
#include "pv/pv_number.h"
#include "pv/pv_string.h"
#include "pv/pv_array.h"
#include "pv/pv_object.h"
#include "pl/pl_dump.h"

#include <stdio.h>

void print_prefix(pl_dump_prefix parts) {
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

pl_dump_prefix pl_dump_new_prefix() {
	pl_dump_prefix parts;
	parts.data = checked_malloc(sizeof(size_t) + sizeof(pl_dump_prefix_part) * 4);
	parts.data->size = 4;
	parts.count = 0;
	return parts;
}

void pl_dump_pv_prefixed(pv val, pl_dump_prefix parts) {
	print_prefix(parts);
	size_t idx;
	pv_kind kind = pv_get_kind(val);
	if (kind == 0) {
  	printf("ERROR OBJECT\n");
  } else if (kind == null_kind) {
  	printf("NULL\n");
  } else if (kind == bool_kind) {
  	if (pv_bool_value(val)) {
  		printf("TRUE\n");
  	} else {
	  	printf("FALSE\n");
  	}
  } else if (kind == number_kind) {
  	printf("%f\n",pv_number_value(val));
  } else if (kind == string_kind) {
  	printf("\"%s\"\n",pv_string_value(val));
  } else if (kind == array_kind) {
  	printf("[]\n");
		inc_size(idx,parts.data,parts.count,sizeof(size_t),parts.data->size,(size_t)((float)parts.data->size * 1.5f));
		pv_array_foreach(val, i, v) {
			parts.data->parts[idx].type = IDX;
			parts.data->parts[idx].idx = i;
			pl_dump_pv_prefixed(v, parts);
  	}
  } else if (kind == object_kind) {
  	printf("{}\n");
		inc_size(idx,parts.data,parts.count,sizeof(size_t),parts.data->size,(size_t)((float)parts.data->size * 1.5f));
		pv_object_foreach(val, k, v) {
			parts.data->parts[idx].type = STR;
			parts.data->parts[idx].str = pv_string_value(k);
			pl_dump_pv_prefixed(v, parts);
  	}
	}
}