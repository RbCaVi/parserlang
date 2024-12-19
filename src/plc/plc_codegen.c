#include "plc_codegen.h"

#include "pv.h"
#include "pv_array.h"
#include "pv_object.h"

#include <stdio.h>
#include <stdlib.h>

struct plc_codegen_context {
	plc_codegen_context *code;
	// i'm using pv here because i'm too lazy to manage c types now
	int *nextglobal; // the next free global index (for inner functions)
	pv globals; // object[name:global index]
	pv vars; // object[name:stack pos] // i can either implement this with a chained object or just copy the parent's vars
	// these have to be passed upward through scopes until resolved
	// unresolved breaks and continues
	// not used yet - i don't have any loops yet
	//pv breaks; // array[{code offset, stack top}] // the stack top is for a popn to restore the stack
	//pv continues; // array[{code offset, stack top}] // the stack top is for a popn to restore the stack
};

plc_codegen_context *plc_codegen_context_new() {
	plc_codegen_context *out = malloc(sizeof(plc_codegen_context));
	pv *globals = malloc(sizeof(pv));
	*globals = pv_object();
	*out = (plc_codegen_context){pl_bytecode_new_builder(), globals, pv_object(), pv_array(), pv_array()};
	return out;
}

plc_codegen_context *plc_codegen_context_chain(plc_codegen_context *c) {
	plc_codegen_context *out = malloc(sizeof(plc_codegen_context));
	pv_copy(*c->globals); // refcount even though this is a pointer - i will free it in plc_codegen_context_free
	*out = (plc_codegen_context){c->globals, pv_copy(c->vars), pv_array(), pv_array()};
	return out;
}

static void plc_codegen_context_add(plc_codegen_context *c, plc_codegen_context *c2) {
	//pv offset = pv_int(pl_bytecode_builder_len(c->code));
	//pv_array_foreach(c2->breaks, i, b) {
	//	c->breaks = pv_array_append(c->breaks, pv_number_add(offset, b));
	//}
	//pv_array_foreach(c2->continues, i, cont) {
	//	c->continues = pv_array_append(c->continues, pv_number_add(offset, cont));
	//}
	plc_codegen_context_free(c2);
}

pl_bytecode_builder *plc_codegen_stmt(plc_codegen_context *c, stmt *s) {
	switch (s->type) {
		case BLOCK: {
			plc_codegen_context *c2 = plc_codegen_context_new();
			for (int i = 0; i < s->block.len; i++) {
				plc_codegen_stmt(c2, &(s->block.children[i]));
				plc_codegen_context_add(c, c2);
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
			plc_codegen_expr(c, s->ret.val);
			pl_bytecode_builder_add(c->code, RET, {});
			break;
		}
		default:
			abort();
	}
	return code;
}

pl_bytecode_builder *plc_codegen_expr(plc_codegen_context *c, expr *e) {
	switch (e->type) {
		case EXPR: {
			printf("EXPR isn't implemented yet :(\n");
			abort();
			break;
		}
		case NUM: {
			pl_bytecode_builder_add(c->code, PUSHINT, {e->n.value});
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

void plc_codegen_context_free(plc_codegen_context *c) {
	// "TODO"
}