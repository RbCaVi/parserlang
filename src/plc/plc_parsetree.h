#ifndef PLC_PARSETREE_H
#define PLC_PARSETREE_H

typedef struct expr expr;

struct expr {
	enum expr_type {
		EXPR,
		NUM,
		SYM,
	} type;
	union {
		struct {
			unsigned int id;
			unsigned int arity;
			expr *children;
		} e;
		struct {
			int value;
		} n;
		struct {
			unsigned int len;
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
		YIELD,
		SET,
	} type;
	union {
		struct {
			unsigned int len;
			stmt *children;
		} block;
		struct {
			unsigned int namelen;
			char *name;
			unsigned int arity;
			unsigned int *arglens;
			char **args;
			stmt *code;
		} deffunc;
		struct {
			expr *cond;
			stmt *code;
		} ifs;
		struct {
			unsigned int namelen;
			char *name;
			expr *val;
		} def;
		struct {
			expr *val;
		} ret;
		struct {
			expr *val;
		} yield;
		struct {
			expr *var; // right now only plain variables are allowed
			expr *val;
		} set;
	};
};

stmt parse_stmt(char *data);

void print_stmt(stmt s, int indent);

void free_stmt(stmt s);

#endif