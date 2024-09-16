#include "pv_array.h"
#include "pv_private.h"
#include "pv_to_string.h"

#include <assert.h>

pv pv_object(void);

pv pv_object_get(pv object, pv key);

int pv_object_has(pv object, pv key);

pv pv_object_set(pv object, pv key, pv value);

pv pv_object_delete(pv object, pv key);

int pv_object_length(pv object);

pv pv_object_merge(pv, pv);

pv pv_object_merge_recursive(pv, pv);

int pv_object_iter(pv);

int pv_object_iter_next(pv, int);

int pv_object_iter_valid(pv, int);

pv pv_object_iter_key(pv, int);

pv pv_object_iter_value(pv, int);