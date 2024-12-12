#include "plc_codegen.h"

#include "pv.h"
#include "pv_array.h"
#include "pv_object.h"

#include <stdio.h>
#include <stdlib.h>

struct plc_codegen_context {
	plc_codegen_context *code;
	// i'm using pv here because i'm too lazy to manage c types now
	pv *globals; // object[name:global index] // pointer so 
	pv vars; // object[name:stack pos] // i can either implement this with a chained object or just copy the parent's vars
	// these have to be passed upward through scopes until resolved
	// unresolved breaks and continues
	// not used yet - i don't have any loops yet
	pv breaks; // array[code offset]
	pv continues; // array[code offset]
};

plc_codegen_context *plc_codegen_context_new() {
	plc_codegen_context *out = malloc(sizeof(plc_codegen_context));
	pv *globals = malloc(sizeof(pv));
	*globals = pv_object();
	*out = (plc_codegen_context){pl_bytecode_new_builder(), globals, pv_object(), pv_array(), pv_array()};
	return out;
}

pl_bytecode_builder *plc_codegen_stmt(plc_codegen_context *c, stmt *s) {
	pl_bytecode_builder *code = pl_bytecode_new_builder();
	switch (s->type) {
		case BLOCK: {
			plc_codegen_context *c2 = plc_codegen_context_new();
			for (int i = 0; i < s->block.len; i++) {
				pl_bytecode_builder *added = plc_codegen_stmt(c2, &(s->block.children[i]));
				pl_bytecode_builder_add_builder(code, added);
				pl_bytecode_builder_free(added);
			}
			break;
		}
		case DEFFUNC: {
			printf("DEFFUNC isn't implemented yet :(\n");
			abort();
			break;
		}
		case IF: {
			printf("IF isn't implemented yet :(\n");
			abort();
			break;
		}
		case DEF: {
			printf("DEF isn't implemented yet :(\n");
			abort();
			break;
		}
		case RETURN: {
			pl_bytecode_builder *added = plc_codegen_expr(c, s->ret.val);
			pl_bytecode_builder_add_builder(code, added);
			pl_bytecode_builder_free(added);
			pl_bytecode_builder_add(code, RET, {});
			break;
		}
		default:
			abort();
	}
	return code;
}

pl_bytecode_builder *plc_codegen_expr(plc_codegen_context *c, expr *e) {
	pl_bytecode_builder *code = pl_bytecode_new_builder();
	switch (e->type) {
		case EXPR: {
			printf("EXPR isn't implemented yet :(\n");
			abort();
			break;
		}
		case NUM: {
			pl_bytecode_builder_add(code, PUSHINT, {e->n.value});
			break;
		}
		case SYM: {
			printf("SYM isn't implemented yet :(\n");
			abort();
			break;
		}
		default:
			abort();
	}
	return code;
}