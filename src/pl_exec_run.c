#include "pv/pv_install.h"
#include "pv/pv_array.h"
#include "pv/pv_to_string.h"

#include "pl/pl_func.h"
#include "pl/pl_iter.h"
#include "pl/pl_bytecode.h"
#include "pl/pl_dump.h"
#include "pl/pl_exec.h"

#include "plc/plc_codegen.h"
#include "plc/plc_parsetree.h"

#include <stdio.h>

#include <emscripten/emscripten.h>

EMSCRIPTEN_KEEPALIVE void run(char *data) {
	pv_install();
	pl_func_install();
	pl_iter_install();

	stmt st = parse_stmt(data);

	//print_stmt(st, 0);

	plc_codegen_context *c = plc_codegen_context_new();

	pl_bytecode_builder *b = plc_codegen_stmt(c, &st);

	//printf("llll bytecode:\n");
	//plp_bytecode_builder_dump(b);
	//printf("aaa bytecode:\n");

	// in case control flow reaches the end, return null
	b = pl_bytecode_dup_builder(b); // the original is freed by plc_codegen_context_free(c)
	pl_bytecode_builder_add(b, GRET, {});
	pl_bytecode code = pl_bytecode_from_builder(b);

	free_stmt(st);

	pv globals = plc_codegen_context_get_globals(c);

	plc_codegen_context_free(c);

	printf("top level bytecode:\n");
	pl_bytecode_dump(code);
	printf("\n");

	pv main = pl_func(code);

	pl_state *pl = pl_state_new();

	int glen = pv_array_length(pv_copy(globals));
	pl->globals = pv_array_data(pv_copy(globals));

	pv_array_foreach(globals, i, v) {
		if (pv_get_kind(v) == func_kind) {
			printf("global %i function bytecode:\n", i);
			pl_bytecode bytecode = pl_func_get_bytecode(pv_copy(v));
			pl_bytecode_dump(bytecode);
			pl_bytecode_free(bytecode);
		} else {
			printf("global %i:\n", i);
			pl_dump_pv(v);
			printf("\n");
		}
	}

	printf("program output:\n");
	pv ret = pl_func_call(main, pl);

	printf("\nreturn value:\n");
	pl_dump_pv(pv_copy(ret));
	char *s = pv_to_string(ret);
	printf("%s\n", s);
	free(s);
	printf("\n");

	pv_free(globals);

	pl_state_free(pl);
}