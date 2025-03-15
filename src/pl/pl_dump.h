#ifndef PL_DUMP_H
#define PL_DUMP_H

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#include "pv/pv.h"
#include "pl/util_pl.h"

typedef struct {
	enum {
		STR,IDX,KEY
	} type;
	union {
		char *str; // STR and KEY
		uint32_t idx;
	};
} pl_dump_prefix_part;

typedef struct {
	struct {
		uint32_t size;
		pl_dump_prefix_part parts[];
	} *data;
	uint32_t count;
} pl_dump_prefix;

pl_dump_prefix pl_dump_new_prefix();
void pl_dump_free_prefix(pl_dump_prefix);
pl_dump_prefix pl_dump_dup_prefix(pl_dump_prefix);

void print_prefix(pl_dump_prefix parts);
#define print_prefixed(parts, ...) print_prefix(parts); printf(__VA_ARGS__); printf("\n");

#define pl_dump_pv(val) {pl_dump_prefix __dump_prefix = pl_dump_new_prefix(); pl_dump_pv_prefixed(val, __dump_prefix); pl_dump_free_prefix(__dump_prefix);}
void pl_dump_pv_prefixed(pv val, pl_dump_prefix parts);

#define pl_dump_prefix_extend(parts) {inc_size3(parts.data,parts.count,sizeof(size_t),parts.data->size,(uint32_t)((float)parts.data->size * 1.5f), parts.data->parts);}
#define pl_dump_prefix_at(parts, idx) parts.data->parts[idx]
#define pl_dump_prefix_set_str(parts, idx_, value) pl_dump_prefix_at(parts, idx_) = (pl_dump_prefix_part){STR, {.str = (value)}}
#define pl_dump_prefix_set_idx(parts, idx_, value) pl_dump_prefix_at(parts, idx_) = (pl_dump_prefix_part){IDX, {.idx = (value)}}
#define pl_dump_prefix_set_key(parts, idx_, value) pl_dump_prefix_at(parts, idx_) = (pl_dump_prefix_part){KEY, {.str = (value)}}

#endif
