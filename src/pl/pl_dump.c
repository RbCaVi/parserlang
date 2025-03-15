#include "pl/pl_dump.h"
#include "pl/util_pl.h"
#include "pv/pv_singletons.h"
#include "pv/pv_number.h"
#include "pv/pv_string.h"
#include "pv/pv_array.h"
#include "pv/pv_object.h"
#include "pl/pl_dump.h"

#include <stdio.h>
#include <string.h>

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

void pl_dump_free_prefix(pl_dump_prefix parts) {
	for (size_t i = 0; i < parts.count; i++) {
		pl_dump_prefix_part part = parts.data->parts[i];
		switch (part.type) {
		case STR:
			free(part.str);
		case IDX:
		case KEY:
			break;
		}
	}
	free(parts.data);
}

pl_dump_prefix pl_dump_dup_prefix(pl_dump_prefix p) {
	pl_dump_prefix parts;
	parts.data = checked_malloc(sizeof(size_t) + sizeof(pl_dump_prefix_part) * p.data->size);
	parts.data->size = p.data->size;
	parts.count = p.count;
	memcpy(parts.data->parts, p.data->parts, sizeof(pl_dump_prefix_part) * p.data->size);
	for (size_t i = 0; i < parts.count; i++) {
		pl_dump_prefix_part *part = &(parts.data->parts[i]);
		switch (part->type) {
		case STR:
			part->str = strdup(part->str);
		case IDX:
		case KEY:
			break;
		}
	}
	return parts;
}

void pl_dump_pv_prefixed(pv val, pl_dump_prefix parts) {
  parts = pl_dump_dup_prefix(parts);
	pv_kind kind = pv_get_kind(val);
	if (kind == 0) {
		print_prefixed(parts, "ERROR OBJECT");
  } else if (kind == null_kind) {
  	print_prefixed(parts, "NULL");
  } else if (kind == bool_kind) {
  	print_prefixed(parts, pv_bool_value(val)?"TRUE":"FALSE");
  } else if (kind == double_kind) {
  	print_prefixed(parts, "%f", pv_double_value(val));
  } else if (kind == int_kind) {
  	print_prefixed(parts, "%i", pv_int_value(val));
  } else if (kind == string_kind) {
  	char *s = pv_string_value(val);
  	print_prefixed(parts, "%i \"%s\"", pv_get_refcount(val), s);
  	free(s);
  } else if (kind == array_kind) {
  	print_prefixed(parts, "%i []", pv_get_refcount(val));
		pl_dump_prefix_extend(parts);
		pl_dump_prefix_set_idx(parts, parts.count - 1, 0); // initialize the part (otherwise conditional move depends on uninitialized value (in pl_dump_free_prefix(parts)))
		pv_array_foreach(val, i, v) {
			pl_dump_prefix_set_idx(parts, parts.count - 1, i);
			pl_dump_pv_prefixed(v, parts);
  	}
  	pv_free(val); // the foreach doesn't free (i need to fix this)
  } else if (kind == object_kind) {
  	print_prefixed(parts, "%i {}", pv_get_refcount(val));
		pl_dump_prefix_extend(parts);
		pl_dump_prefix_set_str(parts, parts.count - 1, NULL); // initialize the part (otherwise conditional move depends on uninitialized value (in pl_dump_free_prefix(parts)))
		pv_object_foreach(val, k, v) {
  		free(pl_dump_prefix_at(parts, parts.count - 1).str);
			pl_dump_prefix_set_str(parts, parts.count - 1, pv_string_value(k));
			pl_dump_pv_prefixed(v, parts);
  	}
		pv_free(val); // the foreach doesn't free (i need to fix this)
	} else {
		print_prefixed(parts, "%i idk it has kind %i (%s) (%p)", pv_get_refcount(val), kind, pv_kind_name(kind), val.data);
  	pv_free(val);
	}
  pl_dump_free_prefix(parts);
}