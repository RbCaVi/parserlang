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

#include <stdio.h>
#include <stdlib.h>

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
	printf("b = %i %i\n", b->end, b->size);
	b = pl_bytecode_builder_add_DUP(b, (pl_DUP_data){});
	printf("b = %i %i\n", b->end, b->size);
	b = pl_bytecode_builder_add_PUSHNUM(b, (pl_PUSHNUM_data){15});
	printf("b = %i %i\n", b->end, b->size);
	b = pl_bytecode_builder_add_RET(b, (pl_RET_data){});
	printf("b = %i %i\n", b->end, b->size);
	pl_bytecode_dump(b->bytecode);
	free(b);
	return 0;
}
