#include "pl_builtins.h"

#include "pv_number.h"
#include "pv_string.h"
#include "pv_array.h"

#include <stdio.h>

pl_builtin pl_builtins[] = {
#define BUILTIN(name) {pl_builtin_ ## name, #name},
#include "pl_builtins_data.h"
#undef BUILTIN
};

pv pl_builtin_len(pl_state *pl) {
	pv val = pl_stack_get(pl->stack, 1);
	pv_kind kind = pv_get_kind(val);
	if (kind == array_kind) {
		return pv_int(pv_array_length(val));
	} else {
		return pv_invalid();
	}
}

pv pl_builtin_strmid(pl_state *pl) {
	pv str = pl_stack_get(pl->stack, 1);
	pv idx = pl_stack_get(pl->stack, 2);
	if (pv_get_kind(str) == string_kind && pv_get_kind(idx) == int_kind ) {
		int i = pv_int_value(idx);
		if (i >= 0) {
			//printf("str refcount a: %i\n", pv_get_refcount(str));
			pv out = pv_string_from_data(pv_string_data(pv_copy(str)) + i, pv_string_length(pv_copy(str)) - i);
			//printf("str refcount b: %i\n", pv_get_refcount(str));
			pv_free(str);
			//printf("str refcount c: %i\n", pv_get_refcount(str));
			return out;
		} else {
			pv_free(str);
		}
	}
	return pv_invalid();
}