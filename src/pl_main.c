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

	// call f2 twice with both inputs and add them together (should return 54)
	pl_bytecode_builder *b = pl_bytecode_new_builder();
	pl_bytecode_builder_add(b, PUSHGLOBAL, {0});
	pl_bytecode_builder_add(b, PUSHBOOL, {0});
	pl_bytecode_builder_add(b, CALL, {1});
	pl_bytecode_builder_add(b, PUSHGLOBAL, {0});
	pl_bytecode_builder_add(b, PUSHBOOL, {1});
	pl_bytecode_builder_add(b, CALL, {1});
	pl_bytecode_builder_add(b, ADD, {});
	pl_bytecode_builder_add(b, RET, {});
	pl_bytecode bytecode = pl_bytecode_from_builder(b);

	printf("bytecode1\n");
	pl_bytecode_dump(bytecode);

	pv f = pl_func(bytecode);

	pl_state *pl = malloc(sizeof(pl_state));
	pl->stack = pl_stack_new();
	pl->globals = malloc(1 * sizeof(pv));

	/* approximately
	f2(b)
		if b: goto l1
		v1 = 8
		v2 = 15
		out = v1 + v2
		v3 = true
		if v3: goto l2
		l1:
		out = 31
		l2:
		return out

	* simplifies to
	f2(b)
		if b:
			return 23
		else:
			return 31
	*/

	pl_bytecode_builder *b2 = pl_bytecode_new_builder();
	pl_bytecode_builder_add(b2, JUMPIF, {44});
	pl_bytecode_builder_add(b2, PUSHNUM, {8});
	pl_bytecode_builder_add(b2, PUSHNUM, {15});
	pl_bytecode_builder_add(b2, ADD, {});
	pl_bytecode_builder_add(b2, PUSHBOOL, {1});
	pl_bytecode_builder_add(b2, JUMPIF, {12});
	pl_bytecode_builder_add(b2, PUSHNUM, {31});
	pl_bytecode_builder_add(b2, RET, {});
	pl_bytecode bytecode2 = pl_bytecode_from_builder(b2);

	printf("bytecode2\n");
	pl_bytecode_dump(bytecode2);

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
