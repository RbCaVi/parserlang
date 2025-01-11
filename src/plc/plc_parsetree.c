#include "plc_parsetree.h"

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

typedef struct {
	char *name;
	unsigned int arity;
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
	NODE_RETURNV,
	NODE_SIG,
	NODE_EXPR,
	NODE_NUM,
	NODE_SYM,
	NODE_YIELD,
	NODE_SETSTMT,
	NODE_FOR,
	NODE_WHILE,
	//NODE_EXPRSTMT,
	//NODE_SETOPSTMT,
} node_type;

typedef struct {
	unsigned int arity;
	plc_sym *args;
} sig;

static expr parse_expr(char *data) {
	node_type type = *(node_type*)data;
	data += sizeof(node_type);
	expr out;
	switch (type) {
	case NODE_EXPR:
		out.type = EXPR_OP;
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
		out.type = EXPR_NUM;
		out.n.value = *(int*)data;
		break;
	case NODE_SYM:
		out.type = EXPR_SYM;
		unsigned int namelen = *(unsigned int*)data;
		out.s.len = namelen;
		data += sizeof(unsigned int);
		out.s.name = data;
		break;
	default:
		abort(); // death
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
	out.args = malloc(sizeof(plc_sym) * arity);
	unsigned int *lens = (unsigned int*)data;
	data += sizeof(int) * arity;
	for (unsigned int i = 0; i < arity; i++) {
		unsigned int len = lens[i];
		out.args[i].name = data;
		out.args[i].len = len;
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
		out.type = STMT_BLOCK;
		unsigned int len = *(unsigned int*)data;
		data += sizeof(unsigned int);
		out.block.len = len;
		out.block.children = malloc(sizeof(stmt) * len);
		int *lens = (int*)data;
		data += sizeof(int) * len;
		for (unsigned int i = 0; i < len; i++) {
			int len = lens[i];
			out.block.children[i] = parse_stmt(data);
			data += len;
		}
		break;
	}
	case NODE_DEFFUNC: {
		out.type = STMT_DEFFUNC;
		unsigned int namelen = *(unsigned int*)data;
		out.deffunc.name.len = namelen;
		data += sizeof(unsigned int);
		int siglen = *(int*)data;
		data += sizeof(int);
		//int codelen = *(int*)data;
		data += sizeof(int);
		out.deffunc.name.name = data;
		data += namelen;
		sig s = parse_sig(data);
		data += siglen;
		out.deffunc.arity = s.arity;
		out.deffunc.args = s.args;
		out.deffunc.code = malloc(sizeof(stmt));
		*out.deffunc.code = parse_stmt(data);
		break;
	}
	case NODE_IF: {
		out.type = STMT_IF;
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
		out.type = STMT_DEF;
		unsigned int namelen = *(unsigned int*)data;
		out.def.name.len = namelen;
		data += sizeof(unsigned int);
		out.def.name.name = data;
		data += namelen;
		out.def.val = malloc(sizeof(expr));
		*out.def.val = parse_expr(data);
		break;
	}
	case NODE_RETURN: {
		out.type = STMT_RETURN;
		break;
	}
	case NODE_RETURNV: {
		out.type = STMT_RETURNV;
		out.retv.val = malloc(sizeof(expr));
		*out.retv.val = parse_expr(data);
		break;
	}
	case NODE_YIELD: {
		out.type = STMT_YIELD;
		out.yield.val = malloc(sizeof(expr));
		*out.yield.val = parse_expr(data);
		break;
	}
	case NODE_SETSTMT: {
		out.type = STMT_SET;
		int varlen = *(int*)data;
		data += sizeof(int);
		//int vallen = *(int*)data;
		data += sizeof(int);
		out.set.var = malloc(sizeof(expr));
		*out.set.var = parse_expr(data);
		data += varlen;
		out.set.val = malloc(sizeof(expr));
		*out.set.val = parse_expr(data);
		break;
	}
	case NODE_FOR: {
		out.type = STMT_FOR;
		unsigned int varlen = *(unsigned int*)data;
		data += sizeof(int);
		int vallen = *(int*)data;
		data += sizeof(int);
		//int codelen = *(int*)data;
		data += sizeof(int);
		out.fors.var.len = varlen;
		out.fors.var.name = data;
		data += varlen;
		out.fors.val = malloc(sizeof(expr));
		*out.fors.val = parse_expr(data);
		data += vallen;
		out.fors.code = malloc(sizeof(stmt));
		*out.fors.code = parse_stmt(data);
		break;
	}
	case NODE_WHILE: {
		out.type = STMT_WHILE;
		int condlen = *(int*)data;
		data += sizeof(int);
		//int codelen = *(int*)data;
		data += sizeof(int);
		out.whiles.cond = malloc(sizeof(expr));
		*out.whiles.cond = parse_expr(data);
		data += condlen;
		out.whiles.code = malloc(sizeof(stmt));
		*out.whiles.code = parse_stmt(data);
		break;
	}
	default:
		abort(); // death
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
	case EXPR_OP:
		print_indent(indent);
		printf("EXPR_OP %s\n", ops[e.e.id].name);
		assert(ops[e.e.id].arity == 0 || ops[e.e.id].arity == e.e.arity);
		for (unsigned int i = 0; i < e.e.arity; i++) {
			print_expr(e.e.children[i], indent + 1);
		}
		break;
	case EXPR_NUM:
		print_indent(indent);
		printf("EXPR_NUM %i\n", e.n.value);
		break;
	case EXPR_SYM:
		print_indent(indent);
		printf("EXPR_SYM %.*s\n", e.s.len, e.s.name);
		break;
	}
}

void print_stmt(stmt s, int indent) {
	switch (s.type) {
	case STMT_BLOCK:
		print_indent(indent);
		printf("STMT_BLOCK\n");
		for (unsigned int i = 0; i < s.block.len; i++) {
			print_stmt(s.block.children[i], indent + 1);
		}
		break;
	case STMT_DEFFUNC:
		print_indent(indent);
		printf("STMT_DEFFUNC %.*s\n", s.deffunc.name.len, s.deffunc.name.name);
		print_indent(indent + 1);
		printf("SIG\n");
		for (unsigned int i = 0; i < s.deffunc.arity; i++) {
			print_indent(indent + 2);
			printf("%.*s\n", s.deffunc.args[i].len, s.deffunc.args[i].name);
		}
		print_stmt(*s.deffunc.code, indent + 1);
		break;
	case STMT_IF:
		print_indent(indent);
		printf("STMT_IF\n");
		print_expr(*s.ifs.cond, indent + 1);
		print_stmt(*s.ifs.code, indent + 1);
		break;
	case STMT_DEF:
		print_indent(indent);
		printf("STMT_DEF %.*s\n", s.def.name.len, s.def.name.name);
		print_expr(*s.def.val, indent + 1);
		break;
	case STMT_RETURN:
		print_indent(indent);
		printf("STMT_RETURN\n");
		break;
	case STMT_RETURNV:
		print_indent(indent);
		printf("STMT_RETURNV\n");
		print_expr(*s.retv.val, indent + 1);
		break;
	case STMT_YIELD:
		print_indent(indent);
		printf("STMT_YIELD\n");
		print_expr(*s.yield.val, indent + 1);
		break;
	case STMT_SET:
		print_indent(indent);
		printf("STMT_SET\n");
		print_expr(*s.set.var, indent + 1);
		print_expr(*s.set.val, indent + 1);
		break;
	case STMT_FOR:
		print_indent(indent);
		printf("STMT_FOR\n");
		print_indent(indent + 1);
		printf("%.*s\n", s.fors.var.len, s.fors.var.name);
		print_expr(*s.fors.val, indent + 1);
		print_stmt(*s.fors.code, indent + 1);
		break;
	case STMT_WHILE:
		print_indent(indent);
		printf("STMT_WHILE\n");
		print_expr(*s.whiles.cond, indent + 1);
		print_stmt(*s.whiles.code, indent + 1);
		break;
	}
}

static void free_expr(expr e) {
	switch (e.type) {
	case EXPR_OP:
		for (unsigned int i = 0; i < e.e.arity; i++) {
			free_expr(e.e.children[i]);
		}
		free(e.e.children);
		break;
	case EXPR_NUM:
		break;
	case EXPR_SYM:
		break;
	}
}

void free_stmt(stmt s) {
	switch (s.type) {
	case STMT_BLOCK:
		for (unsigned int i = 0; i < s.block.len; i++) {
			free_stmt(s.block.children[i]);
		}
		free(s.block.children);
		break;
	case STMT_DEFFUNC:
		free(s.deffunc.args);
		free_stmt(*s.deffunc.code);
		free(s.deffunc.code);
		break;
	case STMT_IF:
		free_expr(*s.ifs.cond);
		free(s.ifs.cond);
		free_stmt(*s.ifs.code);
		free(s.ifs.code);
		break;
	case STMT_DEF:
		free_expr(*s.def.val);
		free(s.def.val);
		break;
	case STMT_RETURN:
		break;
	case STMT_RETURNV:
		free_expr(*s.retv.val);
		free(s.retv.val);
		break;
	case STMT_YIELD:
		free_expr(*s.yield.val);
		free(s.yield.val);
		break;
	case STMT_SET:
		free_expr(*s.set.var);
		free(s.set.var);
		free_expr(*s.set.val);
		free(s.set.val);
		break;
	case STMT_FOR:
		free_expr(*s.fors.val);
		free(s.fors.val);
		free_stmt(*s.fors.code);
		free(s.fors.code);
		break;
	case STMT_WHILE:
		free_expr(*s.whiles.cond);
		free(s.whiles.cond);
		free_stmt(*s.whiles.code);
		free(s.whiles.code);
		break;
	}
}