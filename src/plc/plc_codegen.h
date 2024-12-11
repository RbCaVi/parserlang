#ifndef PLC_CODEGEN_H
#define PLC_CODEGEN_H

#include "plc_parsetree.h"

typedef struct pl_codegen_context pl_codegen_context;

pl_codegen_context *pl_codegen_context_new();

void pl_codegen_stmt(pl_codegen_context *c, stmt *s);
void pl_codegen_expr(pl_codegen_context *c, expr *e);

#endif