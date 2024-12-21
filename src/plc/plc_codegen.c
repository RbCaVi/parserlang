#include "plc_codegen.h"

#include "pv_number.h"
#include "pv_string.h"
#include "pv_array.h"
#include "pv_object.h"
#include "pl_func.h"
#include "pl_dump.h"

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

typedef enum {
#define OP(upper_name, lower_name, op, arity) OP_ ## upper_name,
#include "plc_op_ids.h"
#undef OP
} opcode;

typedef struct {
	opcode op;
	unsigned int arity;
} op;

static op ops[] = {
#define OP(upper_name, lower_name, op, arity) {OP_ ## upper_name, arity},
#include "plc_op_ids.h"
#undef OP
};

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
	*out = (plc_codegen_context){
		pl_bytecode_new_builder(),
		globals,
		globalrefcount,
		pv_object(),
		pv_array(),
		1,
		pv_object()
	};
	return out;
}

plc_codegen_context *plc_codegen_context_chain_scope(plc_codegen_context *c) {
	plc_codegen_context *out = malloc(sizeof(plc_codegen_context));
	(*c->globalrefcount)++;
	*out = (plc_codegen_context){
		pl_bytecode_new_builder(),
		c->globals,
		c->globalrefcount,
		pv_copy(c->globalmap),
		pv_array_append(pv_copy(c->stacktops), pv_int(c->stacksize)),
		c->stacksize,
		pv_copy(c->vars)
	};
	return out;
}

plc_codegen_context *plc_codegen_context_chain(plc_codegen_context *c) {
	plc_codegen_context *out = malloc(sizeof(plc_codegen_context));
	(*c->globalrefcount)++;
	*out = (plc_codegen_context){
		pl_bytecode_new_builder(),
		c->globals,
		c->globalrefcount,
		pv_copy(c->globalmap),
		pv_array(),
		1,
		pv_object()
	};
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
			//printf("collected DEFFUNC: ");
			//pl_dump_pv(pv_string_from_data(s->deffunc.name, s->deffunc.namelen));
			c->globalmap = pv_object_set(c->globalmap, pv_string_from_data(s->deffunc.name, s->deffunc.namelen), pv_int((int)pv_array_length(pv_copy(*c->globals))));
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
	//printf("s->type: %i\n", s->type);
	switch (s->type) {
		case BLOCK: {
			plc_codegen_context *c2 = plc_codegen_context_chain_scope(c);
			for (uint32_t i = 0; i < s->block.len; i++) {
				plc_codegen_stmt_collect_deffunc(c2, &(s->block.children[i]));
			}
			//printf("globals:\n");
			//pl_dump_pv(pv_copy(*c2->globals));
			//pl_dump_pv(pv_copy(c2->globalmap));
			for (uint32_t i = 0; i < s->block.len; i++) {
				plc_codegen_stmt(c2, &(s->block.children[i]));
			}
			plc_codegen_context_add(c, c2);
			pl_bytecode_builder_add(c->code, POPTO, {c->stacksize});
			break;
		}
		case DEFFUNC: {
			plc_codegen_context *c2 = plc_codegen_context_chain(c);
			for (uint32_t i = 0; i < s->deffunc.arity; i++) {
				c2->vars = pv_object_set(c2->vars, pv_string_from_data(s->deffunc.args[i], s->deffunc.arglens[i]), pv_int(c2->stacksize++));
			}
			pl_bytecode_builder *b = plc_codegen_stmt(c2, s->deffunc.code);
			pl_bytecode code = pl_bytecode_from_builder(b);
			plc_codegen_context_free(c2);
			*c->globals = pv_array_set(*c->globals, pv_int_value(pv_object_get(pv_copy(c->globalmap), pv_string_from_data(s->deffunc.name, s->deffunc.namelen))), pl_func(code));
			break;
		}
		case IF: {
			plc_codegen_expr(c, s->ifs.cond);
			plc_codegen_context *c2 = plc_codegen_context_chain_scope(c);
			plc_codegen_stmt(c2, s->ifs.code);
			pl_bytecode_builder_add(c->code, JUMPIF, {8});
			pl_bytecode_builder_add(c->code, JUMP, {(int)pl_bytecode_builder_len(c2->code)});
			plc_codegen_context_add(c, c2);
			break;
		}
		case DEF: {
			plc_codegen_expr(c, s->def.val);
			//printf("vars refcount          %i\n", pv_get_refcount(c->vars));
			c->vars = pv_object_set(c->vars, pv_string_from_data(s->def.name, s->def.namelen), pv_int(c->stacksize++));
			//pl_dump_pv(pv_copy(c->vars));
			break;
		}
		case RETURN: {
			plc_codegen_expr(c, s->ret.val);
			pl_bytecode_builder_add(c->code, RET, {});
			pl_bytecode_builder_add(c->code, GRET, {});
			break;
		}
		case YIELD: {
			plc_codegen_expr(c, s->yield.val);
			pl_bytecode_builder_add(c->code, RET, {});
			break;
		}
		default:
			abort();
	}
	//printf("bytecode:\n");
	//pl_bytecode code = pl_bytecode_from_builder(c->code);
	//pl_bytecode_dump(code);
	//pl_bytecode_free(code);
	return c->code;
}

pl_bytecode_builder *plc_codegen_expr(plc_codegen_context *c, expr *e) {
	switch (e->type) {
		case EXPR: {
			assert(ops[e->e.id].arity == 0 || ops[e->e.id].arity == e->e.arity);
			for (unsigned int i = 0; i < e->e.arity; i++) {
				plc_codegen_expr(c, &(e->e.children[i]));
			}
			switch ((opcode)e->e.id) {
#define UOP(op, op_lower, expr) \
case OP_ ## op: \
	pl_bytecode_builder_add(c->code, op, {}); \
	break;
#define BOP(op, op_lower, expr, isdefault) UOP(op, op_lower, expr)
#include "pv_number_ops_data.h"
#undef UOP
#undef BOP
			case OP_CALL:
				pl_bytecode_builder_add(c->code, CALL, {(int)e->e.arity - 1});
				break;
			case OP_ARRAY:
				pl_bytecode_builder_add(c->code, MAKEARRAY, {(int)e->e.arity});
				break;
			case OP_EQUAL:
				pl_bytecode_builder_add(c->code, EQUAL, {});
				break;
			}
			break;
		}
		case NUM: {
			pl_bytecode_builder_add(c->code, PUSHINT, {e->n.value});
			break;
		}
		case SYM: {
			pv var = pv_string_from_data(e->s.name, e->s.len);
			//pl_dump_pv(pv_copy(c->vars));
			//pl_dump_pv(pv_copy(c->globalmap));
			//pl_dump_pv(pv_copy(var));
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

pv plc_codegen_context_get_globals(plc_codegen_context *c) {
	return pv_copy(*c->globals);
}

void plc_codegen_context_free(plc_codegen_context *c) {
	pl_bytecode_builder_free(c->code);
	(*c->globalrefcount)--;
	if (*c->globalrefcount == 0) {
		pv_free(*c->globals);
		free(c->globals);
		free(c->globalrefcount);
	}
	pv_free(c->globalmap);
	pv_free(c->stacktops);
	pv_free(c->vars);
	free(c);
}