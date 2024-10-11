#include "pv/pv.h"
#include "pv/pv_array.h"
#include "pv/pv_number.h"
#include "pv/pv_singletons.h"
#include "pv/pv_object.h"
#include "pv/pv_string.h"
#include "pv/pv_install.h"

#include "pl/pl_dump.h"
#include "pl/pl_stack.h"
#include "pl/pl_bytecode.h"
#include "pl/pl_exec.h"
#include "pl/pl_func.h"

#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv) {
	(void)argc, (void)argv;
	pv_install();
	pl_func_install();
	pv n = PV_ARRAY(pv_number(15),pv_false(),PV_OBJECT(pv_string("key"),pv_string("value"), pv_string("key2"),pv_string("value2")));
	pl_stack stk = pl_stack_new();
	pl_dump_pv(pv_copy(n));
	stk = pl_stack_push(stk, n);
	pl_dump_stack(stk);
	pl_stack_unref(stk);

	pl_bytecode_builder *b = pl_bytecode_new_builder();
	pl_bytecode_builder_add(b, PUSHGLOBAL, {0});
	pl_bytecode_builder_add(b, CALL, {0});
	pl_bytecode_builder_add(b, RET, {});
	pl_bytecode bytecode = pl_bytecode_from_builder(b);

	pl_bytecode_dump(bytecode);

	pv f = pl_func(bytecode);

	pl_state *pl = malloc(sizeof(pl_state));
	pl->stack = pl_stack_new();
	pl->globals = malloc(1 * sizeof(pv));

	pl_bytecode_builder *b2 = pl_bytecode_new_builder();
	pl_bytecode_builder_add(b2, PUSHNUM, {8});
	pl_bytecode_builder_add(b2, PUSHNUM, {15});
	pl_bytecode_builder_add(b2, ADD, {});
	pl_bytecode_builder_add(b2, RET, {});
	pl_bytecode bytecode2 = pl_bytecode_from_builder(b2);

	pl->globals[0] = pl_func(bytecode2);

	pv ret = pl_func_call(f, pl);
	pl_dump_pv(ret);

	pl_dump_stack(pl->stack);
	pl_stack_unref(pl->stack);

	pv_free(pl->globals[0]);
	free(pl->globals);

	free(pl);

	return 0;
}
