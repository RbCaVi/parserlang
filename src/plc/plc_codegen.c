#include "plc_codegen.h"

#include "pv.h"
#include "pv_number.h"
#include "pv_string.h"
#include "pv_array.h"
#include "pv_object.h"

#include <stdio.h>
#include <stdlib.h>

struct plc_codegen_context {
	pl_bytecode_builder *code;
	// i'm using pv here because i'm too lazy to manage c types now
	pv *globals; // the list of globals (for inner functions mostly)
	int *globalrefcount;
	pv globalmap; // object[name:global index]
	pv stacktops; // array[stack pos]
	int stacksize; // current size of the stack
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
	int *globalrefcount = malloc(sizeof(int));
	*globals = pv_array();
	*globalrefcount = 1;
	*out = (plc_codegen_context){pl_bytecode_new_builder(), globals, globalrefcount, pv_object(), pv_array(), 0, pv_object(), pv_array()};
	return out;
}

plc_codegen_context *plc_codegen_context_chain(plc_codegen_context *c) {
	plc_codegen_context *out = malloc(sizeof(plc_codegen_context));
	pv_copy(*c->globals); // refcount even though this is a pointer - i will free it in plc_codegen_context_free
	(*c->globalrefcount)++;
	*out = (plc_codegen_context){pl_bytecode_new_builder(), c->globals, c->globalrefcount, pv_copy(c->globalmap), pv_array_append(pv_copy(c->stacktops), pv_int(c->stacksize)), c->stacksize, pv_copy(c->vars)};
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
	c->code = pl_bytecode_builder_add_builder(c->code, c2->code);
	plc_codegen_context_free(c2);
}

void plc_codegen_stmt_collect_deffunc(plc_codegen_context *c, stmt *s) {
	// add this deffunc (if it is one) to the scope and globals
	// this is because function definitions are hoisted to the top of their scope
	// usually delimited by brackets
	switch (s->type) {
		case DEFFUNC: {
			c->globalmap = pv_object_set(c->globalmap, pv_string_from_data(s->deffunc.name, s->deffunc.namelen), pv_int(pv_array_length(*c->globals)));
			*c->globals = pv_array_append(*c->globals, pv_invalid());
			break;
		}
		case BLOCK:
		case IF:
		case DEF:
		case RETURN:
			break;
		default:
			abort();
	}
}

pl_bytecode_builder *plc_codegen_stmt(plc_codegen_context *c, stmt *s) {
	switch (s->type) {
		case BLOCK: {
			plc_codegen_context *c2 = plc_codegen_context_chain(c);
			for (int i = 0; i < s->block.len; i++) {
				plc_codegen_stmt_collect_deffunc(c2, &(s->block.children[i]));
			}
			for (int i = 0; i < s->block.len; i++) {
				plc_codegen_stmt(c2, &(s->block.children[i]));
			}
			plc_codegen_context_add(c, c2);
			break;
		}
		case DEFFUNC: {
			printf("DEFFUNC isn't implemented yet :(\n");
			abort();
			break;
		}
		case IF: {
			plc_codegen_expr(c, s->ifs.cond);
			plc_codegen_context *c2 = plc_codegen_context_chain(c);
			plc_codegen_stmt(c2, s->ifs.code);
			pl_bytecode_builder_add(c->code, JUMPIF, {8});
			pl_bytecode_builder_add(c->code, JUMP, {pl_bytecode_builder_len(c2->code)});
			plc_codegen_context_add(c, c2);
			break;
		}
		case DEF: {
			plc_codegen_expr(c, s->def.val);
			c->vars = pv_object_set(c->vars, pv_string_from_data(s->def.name, s->def.namelen), pv_int(c->stacksize++));
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
	//printf("bytecode:\n");
	//pl_bytecode_dump(pl_bytecode_from_builder(pl_bytecode_builder_add_builder(pl_bytecode_new_builder(), c->code)));
	return c->code;
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
			pv var = pv_string_from_data(e->s.name, e->s.len);
			if (pv_object_has(pv_copy(c->vars), pv_copy(var))) {
				pl_bytecode_builder_add(c->code, DUPN, {pv_int_value(pv_object_get(pv_copy(c->vars), var))});
			} else {
				pl_bytecode_builder_add(c->code, PUSHGLOBAL, {pv_int_value(pv_object_get(pv_copy(c->globalmap), var))});
			}
			break;
		}
		default:
			abort();
	}
	return c->code;
}

void plc_codegen_context_free(plc_codegen_context *c) {
	pl_bytecode_builder_free(c->code);
	pv_free(*c->globals);
	(*c->globalrefcount)--;
	if (*c->globalrefcount == 0) {
		free(c->globals);
		free(c->globalrefcount);
	}
	pv_free(c->globalmap);
	pv_free(c->stacktops);
	pv_free(c->vars);
	free(c);
}