#ifndef PLC_PARSETREE_H
#define PLC_PARSETREE_H

typedef struct expr expr;

typedef struct {
	unsigned int len;
	char *name;
} plc_sym;

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
		plc_sym s;
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
		FOR,
	} type;
	union {
		struct {
			unsigned int len;
			stmt *children;
		} block;
		struct {
			plc_sym name;
			plc_sym *args;
			unsigned int arity;
			stmt *code;
		} deffunc;
		struct {
			expr *cond;
			stmt *code;
		} ifs;
		struct {
			plc_sym name;
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
		struct {
			plc_sym var;
			expr *val;
			stmt *code;
		} fors;
	};
};

stmt parse_stmt(char *data);

void print_stmt(stmt s, int indent);

void free_stmt(stmt s);

#endif