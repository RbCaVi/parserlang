#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

typedef struct {
	char *data;
	unsigned int length;
} file_data;

// A function that will read a file at a path into an allocated char pointer buffer 
file_data readfile(char *path) {
	FILE *fptr = fopen(path, "rb"); // Open file for reading
	if (!fptr) {
		abort(); // death
	}
	fseek(fptr, 0, SEEK_END); // Seek to the end of the file
	unsigned int length = (unsigned int)ftell(fptr); // Find out how many bytes into the file we are
	char *buf = (char*)malloc(length); // Allocate a buffer for the length of the file
	fseek(fptr, 0, SEEK_SET); // Go back to the beginning of the file
	fread(buf, length, 1, fptr); // Read the contents of the file into the buffer
	fclose(fptr); // Close the file

	return (file_data){buf, length}; // Return the buffer
}

//#include "plc_op_ids.h"

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

typedef struct expr expr;

struct expr {
	enum expr_type {
		EXPR,
		NUM,
		SYM,
	} type;
	union {
		struct {
			int id;
			int arity;
			expr *children;
		} e;
		struct {
			int value;
		} n;
		struct {
			int len;
			char *name;
		} s;
	};
};

typedef struct stmt stmt;

struct stmt {
	enum stmt_type {
		BLOCK,
		DEFFUNC,
		IF,
		DEF,
		RETURN,
	} type;
	union {
		struct {
			int len;
			stmt *children;
		} block;
		struct {
			int namelen;
			char *name;
			int arity;
			int *arglens;
			char **args;
			stmt *code;
		} deffunc;
		struct {
			expr *cond;
			stmt *code;
		} ifs;
		struct {
			int namelen;
			char *name;
			expr *val;
		} def;
		struct {
			expr *val;
		} ret;
	};
};

typedef struct {
	int arity;
	int *arglens;
	char **args;
} sig;

expr parse_expr(char *data) {
	node_type type = *(node_type*)data;
	data += sizeof(node_type);
	expr out;
	switch (type) {
	case NODE_EXPR:
		out.type = EXPR;
		int opid = *(int*)data;
		data += sizeof(int);
		out.e.id = opid;
		int arity = *(int*)data;
		data += sizeof(int);
		out.e.arity = arity;
		out.e.children = malloc(sizeof(expr) * arity);
		int *lens = (int*)data;
		data += sizeof(int) * arity;
		for (int i = 0; i < arity; i++) {
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
		int namelen = *(int*)data;
		out.s.len = namelen;
		data += sizeof(int);
		out.s.name = data;
		break;
	default:
		assert(false);
	}
	return out;
}

sig parse_sig(char *data) {
	node_type type = *(node_type*)data;
	data += sizeof(node_type);
	assert(type == NODE_SIG);
	sig out;
	int arity = *(int*)data;
	data += sizeof(int);
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
		int len = *(int*)data;
		data += sizeof(int);
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
		int namelen = *(int*)data;
		out.deffunc.namelen = namelen;
		data += sizeof(int);
		int siglen = *(int*)data;
		data += sizeof(int);
		int codelen = *(int*)data;
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
		int codelen = *(int*)data;
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