#include "pl_exec.h"
#include "pl_dump.h"

#include <stdio.h>

pv pt_dump_pv(pl_state *pl) {
	pv val = pl_stack_top(pl->stack);
	printf("dumping a pv:\n");
	pl_dump_pv(pv_copy(val));
	return val;
}