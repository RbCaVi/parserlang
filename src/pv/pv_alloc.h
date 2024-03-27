#ifndef PV_ALLOC_H
#define PV_ALLOC_H

#include <stddef.h>
#include "pv.h"

void* pv_mem_alloc(size_t);
void* pv_mem_alloc_unguarded(size_t);
void* pv_mem_calloc(size_t, size_t);
void* pv_mem_calloc_unguarded(size_t, size_t);
char* pv_mem_strdup(const char *);
char* pv_mem_strdup_unguarded(const char *);
void pv_mem_free(void*);
__attribute__((warn_unused_result)) void* pv_mem_realloc(void*, size_t);

#endif
