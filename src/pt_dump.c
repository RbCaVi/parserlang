#include "pl_exec.h"
#include "pl_dump.h"
#include "pv_to_string.h"

#include <stdio.h>
#include <stdlib.h>

pv pt_dump_pv(pl_state *pl) {
	pv val = pl_stack_top(pl->stack);
	printf("dumping a pv:\n");
	pl_dump_pv(pv_copy(val));
	char *s = pv_to_string(pv_copy(val));
	printf("%s\n", s);
	free(s);
	return val;
}