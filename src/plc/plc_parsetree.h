#ifndef PLC_PARSETREE_H
#define PLC_PARSETREE_H

typedef struct expr expr;

typedef struct {
	unsigned int len;
	char *name;
} plc_sym;

struct expr {
	enum expr_type {
		EXPR_OP,
		EXPR_INT,
		EXPR_FLOAT,
		EXPR_STR,
		EXPR_SYM,
	} type;
	union {
		struct {
			unsigned int id;
			unsigned int arity;
			expr *children;
		} e;
		struct {
			int value;
		} i;
		struct {
			float value;
		} f;
		plc_sym s; // i'm reusing this for both sym and str
	};
};

typedef struct stmt stmt;

struct stmt {
	enum stmt_type {
		STMT_BLOCK,
		STMT_DEFFUNC,
		STMT_IF,
		STMT_DEF,
		STMT_RETURN,
		STMT_RETURNV,
		STMT_YIELD,
		STMT_SET,
		STMT_FOR,
		STMT_WHILE,
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
		} ret;
		struct {
			expr *val;
		} retv;
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
		struct {
			expr *cond;
			stmt *code;
		} whiles;
	};
};

stmt parse_stmt(char *data);

void print_stmt(stmt s, int indent);

void free_stmt(stmt s);

#endif