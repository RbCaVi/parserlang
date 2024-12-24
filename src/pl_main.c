#include "pv/pv.h"
#include "pv/pv_array.h"
#include "pv/pv_number.h"
#include "pv/pv_singletons.h"
#include "pv/pv_object.h"
#include "pv/pv_string.h"
#include "pv/pv_install.h"
#include "pv/pv_to_string.h"

#include "pl/pl_dump.h"
#include "pl/pl_stack.h"
#include "pl/pl_bytecode.h"
#include "pl/pl_exec.h"
#include "pl/pl_func.h"
#include "pl/pl_iter.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

pv dump_pv(pl_state *pl) {
	pv val = pl_stack_top(pl->stack);
	printf("dumping a pv:\n");
	pl_dump_pv(pv_copy(val));
	return val;
}

int main(int argc, char **argv) {
	(void)argc, (void)argv;
	pv_install();
	pl_func_install();
	pl_iter_install();
	pv n = PV_ARRAY(pv_double(15),pv_int(15),pv_false(),PV_OBJECT(pv_string("key"),pv_string("value"), pv_string("key2"),pv_string("value2")));
	pl_stack stk = pl_stack_new();
	pl_dump_pv(pv_copy(n));
	stk = pl_stack_push(stk, n);
	pl_dump_stack(stk);
	pl_stack_unref(stk);

	{
		// call f2 twice with both inputs and make an array (should return [23, 31])
		pl_bytecode_builder *b = pl_bytecode_new_builder();
		pl_bytecode_builder_add(b, PUSHGLOBAL, {0});
		pl_bytecode_builder_add(b, PUSHBOOL, {0});
		pl_bytecode_builder_add(b, CALL, {1});
		pl_bytecode_builder_add(b, PUSHGLOBAL, {0});
		pl_bytecode_builder_add(b, PUSHBOOL, {1});
		pl_bytecode_builder_add(b, CALL, {1});
		pl_bytecode_builder_add(b, MAKEARRAY, {2});
		pl_bytecode_builder_add(b, PUSHBOOL, {0});
		pl_bytecode_builder_add(b, APPEND, {});
		pl_bytecode_builder_add(b, RET, {});
		pl_bytecode bytecode = pl_bytecode_from_builder(b);

		printf("bytecode1\n");
		pl_bytecode_dump(bytecode);

		pv f = pl_func(bytecode);

		pl_state *pl = pl_state_new();
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
				return 31
			else:
				return 23
		*/

		pl_bytecode_builder *b2 = pl_bytecode_new_builder();
		pl_bytecode_builder_add(b2, JUMPIF, {44});
		pl_bytecode_builder_add(b2, PUSHDOUBLE, {8});
		pl_bytecode_builder_add(b2, PUSHDOUBLE, {15});
		pl_bytecode_builder_add(b2, ADD, {});
		pl_bytecode_builder_add(b2, PUSHBOOL, {1});
		pl_bytecode_builder_add(b2, JUMPIF, {12});
		pl_bytecode_builder_add(b2, PUSHDOUBLE, {31});
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
	}
	{
		pv a = PV_ARRAY(pv_int(1), pv_int(4), pv_int(2));

		pv ii = pl_iter(a);

		for (int i = 0; i < 5; i++) {
			char *s = pv_to_string(pl_iter_value(pv_copy(ii)));
			printf("iter %i: %s\n", i, s);
			free(s);
			ii = pl_iter_next(ii);
		}

		pv_free(ii);
	}
	{
		pv o = PV_OBJECT(pv_string("k"), pv_int(4), pv_string("k2"), pv_int(2));

		pv ii = pl_iter_keys(pv_copy(o));
		pv ii2 = pl_iter_values(o);

		for (int i = 0; i < 5; i++) {
			char *s = pv_to_string(pl_iter_value(pv_copy(ii)));
			char *s2 = pv_to_string(pl_iter_value(pv_copy(ii2)));
			printf("iter %i: %s %s\n", i, s, s2);
			free(s);
			free(s2);
			ii = pl_iter_next(ii);
			ii2 = pl_iter_next(ii2);
		}

		pv_free(ii);
		pv_free(ii2);
	}
	{
		pl_bytecode_builder *b = pl_bytecode_new_builder();
		pl_bytecode_builder_add(b, PUSHDOUBLE, {8});
		pl_bytecode_builder_add(b, PUSHDOUBLE, {15});
		pl_bytecode_builder_add(b, MAKEARRAY, {2}); // a
		pl_bytecode_builder_add(b, DUP, {}); // a a
		pl_bytecode_builder_add(b, MAKEARRAY, {0}); // a a []
		pl_bytecode_builder_add(b, SWAPN, {2}); // a [] a
		pl_bytecode_builder_add(b, ITERK, {}); // a [] i
		pl_bytecode_builder_add(b, ITERATE, {44}); // a [] i v
		pl_bytecode_builder_add(b, SWAPN, {3}); // a v i []
		pl_bytecode_builder_add(b, SWAPN, {2}); // a v [] i
		pl_bytecode_builder_add(b, SWAPN, {3}); // a i [] v
		pl_bytecode_builder_add(b, APPEND, {}); // a i [v]
		pl_bytecode_builder_add(b, SWAPN, {2}); // a [v] i
		pl_bytecode_builder_add(b, JUMP, {-52});
		pl_bytecode_builder_add(b, SWAPN, {2}); // a [] a
		pl_bytecode_builder_add(b, DUP, {}); // a a
		pl_bytecode_builder_add(b, MAKEARRAY, {0}); // a a []
		pl_bytecode_builder_add(b, SWAPN, {2}); // a [] a
		pl_bytecode_builder_add(b, ITERV, {}); // a [] i
		pl_bytecode_builder_add(b, ITERATE, {44}); // a [] i v
		pl_bytecode_builder_add(b, SWAPN, {3}); // a v i []
		pl_bytecode_builder_add(b, SWAPN, {2}); // a v [] i
		pl_bytecode_builder_add(b, SWAPN, {3}); // a i [] v
		pl_bytecode_builder_add(b, APPEND, {}); // a i [v]
		pl_bytecode_builder_add(b, SWAPN, {2}); // a [v] i
		pl_bytecode_builder_add(b, JUMP, {-52});
		pl_bytecode_builder_add(b, SWAPN, {2}); // a [] a
		pl_bytecode_builder_add(b, DUP, {}); // a a
		pl_bytecode_builder_add(b, MAKEARRAY, {0}); // a a []
		pl_bytecode_builder_add(b, SWAPN, {2}); // a [] a
		pl_bytecode_builder_add(b, ITERE, {}); // a [] i
		pl_bytecode_builder_add(b, ITERATE, {44}); // a [] i v
		pl_bytecode_builder_add(b, SWAPN, {3}); // a v i []
		pl_bytecode_builder_add(b, SWAPN, {2}); // a v [] i
		pl_bytecode_builder_add(b, SWAPN, {3}); // a i [] v
		pl_bytecode_builder_add(b, APPEND, {}); // a i [v]
		pl_bytecode_builder_add(b, SWAPN, {2}); // a [v] i
		pl_bytecode_builder_add(b, JUMP, {-52});
		pl_bytecode_builder_add(b, SWAPN, {2}); // a [] a
		pl_bytecode_builder_add(b, POP, {});
		pl_bytecode_builder_add(b, MAKEARRAY, {2});
		pl_bytecode_builder_add(b, RET, {});
		pl_bytecode bytecode = pl_bytecode_from_builder(b);

		printf("bytecode1\n");
		pl_bytecode_dump(bytecode);

		pv f = pl_func(bytecode);

		pl_state *pl = pl_state_new();
		pl->globals = malloc(0 * sizeof(pv));

		pv ret = pl_func_call(f, pl);
		pl_dump_pv(ret);

		pl_dump_stack(pl->stack);
		pl_stack_unref(pl->stack);

		free(pl->globals);

		free(pl);
	}
	{
		pv f = pl_func_native(dump_pv);

		pl_state *pl = pl_state_new();

		pl->stack = pl_stack_push(pl->stack, PV_ARRAY(pv_int(15)));

		pv ret = pl_func_call(f, pl);
		pl_dump_pv(ret);

		pl_dump_stack(pl->stack);
		pl_stack_unref(pl->stack);

		free(pl);
	}
	{
		pv f = pl_func_from_symbol("./pt_dump.so", "pt_dump_pv");

		pl_state *pl = pl_state_new();

		pl->stack = pl_stack_push(pl->stack, PV_ARRAY(pv_int(15)));

		pv ret = pl_func_call(f, pl);
		pl_dump_pv(ret);

		pl_dump_stack(pl->stack);
		pl_stack_unref(pl->stack);

		free(pl);
	}
	{
		pl_bytecode_builder *b = pl_bytecode_new_builder();
		pl_bytecode_builder_add(b, PUSHDOUBLE, {8});
		pl_bytecode_builder_add(b, PUSHDOUBLE, {15});
		pl_bytecode_builder_add(b, RET, {});
		pl_bytecode_builder_add(b, RET, {});
		pl_bytecode bytecode = pl_bytecode_from_builder(b);

		printf("bytecode1\n");
		pl_bytecode_dump(bytecode);

		pv f = pl_func(bytecode);

		pl_state *pl = pl_state_new();
		pl->globals = malloc(0 * sizeof(pv));

		pl->stack = pl_stack_push(pl->stack, f);
		pl_state_set_call(pl, 0);

		pv ret1 = pl_next(pl);
		pl_dump_pv(ret1);

		pv ret2 = pl_next(pl);
		pl_dump_pv(ret2);

		pl_dump_stack(pl->stack);
		pl_stack_unref(pl->stack);

		free(pl->globals);

		free(pl);
	}
	{
		pl_bytecode_builder *b = pl_bytecode_new_builder();
		pl_bytecode_builder_add(b, PUSHDOUBLE, {8});
		pl_bytecode_builder_add(b, PUSHDOUBLE, {15});
		pl_bytecode_builder_add(b, RET, {});
		pl_bytecode_builder_add(b, RET, {});
		pl_bytecode_builder_add(b, GRET, {});
		pl_bytecode bytecode = pl_bytecode_from_builder(b);

		printf("bytecode1\n");
		pl_bytecode_dump(bytecode);

		pv f = pl_func(bytecode);

		pl_state *pl = pl_state_new();

		pl->stack = pl_stack_push(pl->stack, f);
		pl_state_set_call(pl, 0);

		pv ii = pl_iter_gen(pl);

		for (int i = 0; i < 5; i++) {
			char *s = pv_to_string(pl_iter_value(pv_copy(ii)));
			printf("iter %i: %s\n", i, s);
			free(s);
			ii = pl_iter_next(ii);
		}

		pv_free(ii);

		pl_dump_stack(pl->stack);
		pl_stack_unref(pl->stack);

		free(pl);
	}

	return 0;
}
