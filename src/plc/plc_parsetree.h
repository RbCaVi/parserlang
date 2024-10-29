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

stmt parse_stmt(char *data);

void print_stmt(stmt s, int indent);

void free_stmt(stmt s);