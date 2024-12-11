#include "plc_codegen.h"

#include <stdio.h>
#include <stdlib.h>

struct pl_codegen_context {
	pl_bytecode_builder *code;
	// i'm using pv here because i'm too lazy to manage c types now
	pv vars; // object[str:stack pos] // i can either implement this with a chained object or just "copy" the parent's map
	// these have to be passed upward through scopes until resolved
	pv unresolved; // object[str:array[stack pos]]
	// not used yet - i don't have any loops yet
	// the code offset actually has to be bytecode + offset for unresolved variables in inner functions
	pv breaks; // array[code offset]
	pv continues; // array[code offset]
};

pl_codegen_context *pl_codegen_context_new() {
	pl_codegen_context *out = malloc(sizeof(pl_codegen_context));
	*out = (pl_codegen_context){pl_bytecode_builder_new(), pv_object(), pv_object(), pv_array(), pv_array()};
	return out;
}

void pl_codegen_stmt(pl_codegen_context *c, stmt *s) {
	switch (s->type) {
		case BLOCK: {
			printf("BLOCK isn't implemented yet :(\n");
			abort();
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
			printf("RETURN isn't implemented yet :(\n");
			abort();
			break;
		}
		default:
			abort();
	}
}

void pl_codegen_expr(pl_codegen_context *c, expr *e) {
	switch (e->type) {
		case EXPR: {
			printf("EXPR isn't implemented yet :(\n");
			abort();
			break;
		}
		case NUM: {
			printf("NUM isn't implemented yet :(\n");
			abort();
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
}