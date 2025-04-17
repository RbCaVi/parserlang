#include "pl_builtins.h"

#include "pv_number.h"
#include "pv_string.h"
#include "pv_array.h"
#include "pv_object.h"
#include "pv_to_string.h"

#include <stdio.h>
#include <stdlib.h>

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
	} else if (kind == string_kind) {
		return pv_int(pv_string_length(val));
	} else {
		pv_free(val);
		return pv_invalid();
	}
}

pv pl_builtin_strmid(pl_state *pl) {
	pv str = pl_stack_get(pl->stack, 1);
	pv idx = pl_stack_get(pl->stack, 2);
	if (pv_get_kind(str) == string_kind && pv_get_kind(idx) == int_kind ) {
		int i = pv_int_value(idx);
		if (i >= 0) {
			pv out = pv_string_from_data(pv_string_data(pv_copy(str)) + i, pv_string_length(pv_copy(str)) - i);
			pv_free(str);
			return out;
		}
	}
	pv_free(str);
	pv_free(idx);
	return pv_invalid();
}

pv pl_builtin_strleft(pl_state *pl) {
	pv str = pl_stack_get(pl->stack, 1);
	pv idx = pl_stack_get(pl->stack, 2);
	if (pv_get_kind(str) == string_kind && pv_get_kind(idx) == int_kind) {
		int i = pv_int_value(idx);
		if (i >= pv_string_length(pv_copy(str))) {
			return str;
		} else if (i >= 0) {
			pv out = pv_string_from_data(pv_string_data(pv_copy(str)), i);
			pv_free(str);
			return out;
		} else {
			pv_free(str);
		}
	}
	pv_free(str);
	pv_free(idx);
	return pv_invalid();
}

pv pl_builtin_ord(pl_state *pl) {
	pv str = pl_stack_get(pl->stack, 1);
	if (pv_get_kind(str) == string_kind) {
		pv out = pv_int(pv_string_data(pv_copy(str))[0]);
		pv_free(str);
		return out;
	}
	pv_free(str);
	return pv_invalid();
}

pv pl_builtin_chr(pl_state *pl) {
	pv num = pl_stack_get(pl->stack, 1);
	if (pv_get_kind(num) == int_kind) {
		char data = (char)pv_int_value(num);
		pv out = pv_string_from_data(&data, 1);
		return out;
	}
	pv_free(num);
	return pv_invalid();
}

pv pl_builtin_strcat(pl_state *pl) {
	pv s1 = pl_stack_get(pl->stack, 1);
	pv s2 = pl_stack_get(pl->stack, 2);
	pl->stack = pl_stack_popto(pl->stack, 1);
	if (pv_get_kind(s1) == string_kind && pv_get_kind(s1) == string_kind) {
		return pv_string_concat(s1, s2);
	}
	pv_free(s1);
	pv_free(s2);
	return pv_invalid();
}

pv pl_builtin_print(pl_state *pl) {
	pv v = pl_stack_get(pl->stack, 1);
	printf("%i ", pv_get_refcount(v));
	char *s = pv_to_string(v);
	printf("%s\n", s);
	free(s);
	return pv_invalid();
}

pv pl_builtin_object(pl_state *pl) {
	return pv_object();
}