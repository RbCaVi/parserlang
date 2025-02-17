#include "pl_builtins.h"

#include "pv_number.h"
#include "pv_array.h"

pl_builtin pl_builtins[] = {
#define BUILTIN(pl_name, c_name) {c_name, #pl_name},
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