#ifndef PL_DUMP_H
#define PL_DUMP_H

#include <stddef.h>

#include "pv/pv.h"
#include "pl/pl_stack.h"

typedef struct {
	enum {
		STR,IDX,KEY
	} type;
	union {
		const char *str; // STR and KEY
		int idx;
	};
} pl_dump_prefix_part;

typedef struct {
	struct {
		size_t size;
	  pl_dump_prefix_part parts[];
	} *data;
	size_t count;
} pl_dump_prefix;

void pl_dump_pv(pv val);
void pl_dump_pv_prefixed(pv val, pl_dump_prefix parts);

void pl_dump_stack(pl_stack);
void pl_dump_stack_prefixed(pl_stack, pl_dump_prefix parts);

#endif
