#include "plc_parsetree.h"

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

typedef struct {
	char *name;
	int arity;
} op;

static op ops[] = {
#define OP(upper_name, lower_name, op, arity) {op, arity},
#include "plc_op_ids.h"
#undef OP
};

typedef enum {
	NODE_BLOCK,
	NODE_DEFFUNC,
	NODE_IF,
	NODE_DEF,
	NODE_RETURN,
	NODE_SIG,
	NODE_EXPR,
	NODE_NUM,
	NODE_SYM,
} node_type;

typedef struct {
	unsigned int arity;
	unsigned int *arglens;
	char **args;
} sig;

static expr parse_expr(char *data) {
	node_type type = *(node_type*)data;
	data += sizeof(node_type);
	expr out;
	switch (type) {
	case NODE_EXPR:
		out.type = EXPR;
		unsigned int opid = *(unsigned int*)data;
		data += sizeof(unsigned int);
		out.e.id = opid;
		unsigned int arity = *(unsigned int*)data;
		data += sizeof(unsigned int);
		out.e.arity = arity;
		out.e.children = malloc(sizeof(expr) * arity);
		int *lens = (int*)data;
		data += sizeof(int) * arity;
		for (unsigned int i = 0; i < arity; i++) {
			int len = lens[i];
			out.e.children[i] = parse_expr(data);
			data += len;
		}
		break;
	case NODE_NUM:
		out.type = NUM;
		out.n.value = *(int*)data;
		break;
	case NODE_SYM:
		out.type = SYM;
		unsigned int namelen = *(unsigned int*)data;
		out.s.len = namelen;
		data += sizeof(unsigned int);
		out.s.name = data;
		break;
	default:
		assert(false);
	}
	return out;
}

static sig parse_sig(char *data) {
	node_type type = *(node_type*)data;
	data += sizeof(node_type);
	assert(type == NODE_SIG);
	sig out;
	unsigned int arity = *(unsigned int*)data;
	data += sizeof(unsigned int);
	out.arity = arity;
	out.args = malloc(sizeof(char*) * arity);
	int *lens = (int*)data;
	data += sizeof(int) * arity;
	out.arglens = lens;
	for (int i = 0; i < arity; i++) {
		int len = lens[i];
		out.args[i] = data;
		data += len;
	}
	return out;
}

stmt parse_stmt(char *data) {
	node_type type = *(node_type*)data;
	data += sizeof(node_type);
	stmt out;
	switch (type) {
	case NODE_BLOCK: {
		out.type = BLOCK;
		unsigned int len = *(unsigned int*)data;
		data += sizeof(unsigned int);
		out.block.len = len;
		out.block.children = malloc(sizeof(stmt) * len);
		int *lens = (int*)data;
		data += sizeof(int) * len;
		for (int i = 0; i < len; i++) {
			int len = lens[i];
			out.block.children[i] = parse_stmt(data);
			data += len;
		}
		break;
	}
	case NODE_DEFFUNC: {
		out.type = DEFFUNC;
		unsigned int namelen = *(unsigned int*)data;
		out.deffunc.namelen = namelen;
		data += sizeof(unsigned int);
		int siglen = *(int*)data;
		data += sizeof(int);
		//int codelen = *(int*)data;
		data += sizeof(int);
		out.deffunc.name = data;
		data += namelen;
		sig s = parse_sig(data);
		data += siglen;
		out.deffunc.arity = s.arity;
		out.deffunc.arglens = s.arglens;
		out.deffunc.args = s.args;
		out.deffunc.code = malloc(sizeof(stmt));
		*out.deffunc.code = parse_stmt(data);
		break;
	}
	case NODE_IF: {
		out.type = IF;
		int condlen = *(int*)data;
		data += sizeof(int);
		//int codelen = *(int*)data;
		data += sizeof(int);
		out.ifs.cond = malloc(sizeof(expr));
		*out.ifs.cond = parse_expr(data);
		data += condlen;
		out.ifs.code = malloc(sizeof(stmt));
		*out.ifs.code = parse_stmt(data);
		break;
	}
	case NODE_DEF: {
		out.type = DEF;
		int namelen = *(int*)data;
		out.def.namelen = namelen;
		data += sizeof(int);
		out.def.name = data;
		data += namelen;
		out.def.val = malloc(sizeof(expr));
		*out.def.val = parse_expr(data);
		break;
	}
	case NODE_RETURN: {
		out.type = RETURN;
		out.ret.val = malloc(sizeof(expr));
		*out.ret.val = parse_expr(data);
		break;
	}
	default:
		assert(false);
	}
	return out;
}

static void print_indent(int indent) {
	for (int i = 0; i < indent; i++) {
		printf("  ");
	}
}

static void print_expr(expr e, int indent) {
	switch (e.type) {
	case EXPR:
		print_indent(indent);
		printf("EXPR %s\n", ops[e.e.id].name);
		assert(ops[e.e.id].arity == 0 || ops[e.e.id].arity == e.e.arity);
		for (int i = 0; i < e.e.arity; i++) {
			print_expr(e.e.children[i], indent + 1);
		}
		break;
	case NUM:
		print_indent(indent);
		printf("NUM %i\n", e.n.value);
		break;
	case SYM:
		print_indent(indent);
		printf("SYM %.*s\n", e.s.len, e.s.name);
		break;
	}
}

void print_stmt(stmt s, int indent) {
	switch (s.type) {
	case BLOCK:
		print_indent(indent);
		printf("BLOCK\n");
		for (int i = 0; i < s.block.len; i++) {
			print_stmt(s.block.children[i], indent + 1);
		}
		break;
	case DEFFUNC:
		print_indent(indent);
		printf("DEFFUNC %.*s\n", s.deffunc.namelen, s.deffunc.name);
		print_indent(indent + 1);
		printf("SIG\n");
		for (int i = 0; i < s.deffunc.arity; i++) {
			print_indent(indent + 2);
			printf("%.*s\n", s.deffunc.arglens[i], s.deffunc.args[i]);
		}
		print_stmt(*s.deffunc.code, indent + 1);
		break;
	case IF:
		print_indent(indent);
		printf("IF\n");
		print_expr(*s.ifs.cond, indent + 1);
		print_stmt(*s.ifs.code, indent + 1);
		break;
	case DEF:
		print_indent(indent);
		printf("DEF %.*s\n", s.def.namelen, s.def.name);
		print_expr(*s.def.val, indent + 1);
		break;
	case RETURN:
		print_indent(indent);
		printf("RETURN\n");
		print_expr(*s.ret.val, indent + 1);
		break;
	}
}

static void free_expr(expr e) {
	switch (e.type) {
	case EXPR:
		for (int i = 0; i < e.e.arity; i++) {
			free_expr(e.e.children[i]);
		}
		free(e.e.children);
		break;
	case NUM:
		break;
	case SYM:
		break;
	}
}

void free_stmt(stmt s) {
	switch (s.type) {
	case BLOCK:
		for (int i = 0; i < s.block.len; i++) {
			free_stmt(s.block.children[i]);
		}
		free(s.block.children);
		break;
	case DEFFUNC:
		free(s.deffunc.args);
		free_stmt(*s.deffunc.code);
		free(s.deffunc.code);
		break;
	case IF:
		free_expr(*s.ifs.cond);
		free(s.ifs.cond);
		free_stmt(*s.ifs.code);
		free(s.ifs.code);
		break;
	case DEF:
		free_expr(*s.def.val);
		free(s.def.val);
		break;
	case RETURN:
		free_expr(*s.ret.val);
		free(s.ret.val);
		break;
	}
}