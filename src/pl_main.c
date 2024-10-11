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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char **argv) {
	(void)argc, (void)argv;
	pv_install();
	pv n = PV_ARRAY(pv_number(15),pv_false(),PV_OBJECT(pv_string("key"),pv_string("value"), pv_string("key2"),pv_string("value2")));
	pl_stack stk = pl_stack_new();
	pl_dump_pv(pv_copy(n));
	stk = pl_stack_push(stk, n);
	pl_dump_stack(stk);
	pl_stack_unref(stk);

	pl_bytecode_builder *b = pl_bytecode_new_builder();
	pl_bytecode_builder_add(b, PUSHNUM, {8});
	pl_bytecode_builder_add(b, PUSHNUM, {15});
	pl_bytecode_builder_add(b, ADD, {});
	pl_bytecode_builder_add(b, RET, {});
	pl_bytecode_dump(b->bytecode);

	char *bytecode = malloc(b->end);
	memcpy(bytecode, b->bytecode, b->end);
	free(b);

	pl_func f = {bytecode};

	pl_state *pl = malloc(sizeof(pl_state));
	pl->stack = pl_stack_new();

	pv ret = pl_call(pl, f);
	pl_dump_pv(ret);

	pl_dump_stack(pl->stack);
	pl_stack_unref(pl->stack);

	free(pl);

	free(bytecode);

	return 0;
}
