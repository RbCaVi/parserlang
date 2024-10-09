#ifndef PL_DUMP_H
#define PL_DUMP_H

#include <stddef.h>
#include <stdint.h>

#include "pv/pv.h"

typedef struct {
	enum {
		STR,IDX,KEY
	} type;
	union {
		const char *str; // STR and KEY
		uint32_t idx;
	};
} pl_dump_prefix_part;

typedef struct {
	struct {
		size_t size;
	  pl_dump_prefix_part parts[];
	} *data;
	size_t count;
} pl_dump_prefix;

pl_dump_prefix pl_dump_new_prefix();
void pl_dump_free_prefix(pl_dump_prefix);
pl_dump_prefix pl_dump_dup_prefix(pl_dump_prefix);

void print_prefix(pl_dump_prefix parts);

#define pl_dump_pv(val) {pl_dump_prefix __dump_prefix = pl_dump_new_prefix(); pl_dump_pv_prefixed(val, __dump_prefix); pl_dump_free_prefix(__dump_prefix);}
void pl_dump_pv_prefixed(pv val, pl_dump_prefix parts);

#endif
