#ifndef PLC_CODEGEN_H
#define PLC_CODEGEN_H

#include "pl_bytecode.h"
#include "plc_parsetree.h"

typedef struct plc_codegen_context plc_codegen_context;

plc_codegen_context *plc_codegen_context_new();
plc_codegen_context *plc_codegen_context_chain(plc_codegen_context*);

pl_bytecode_builder *plc_codegen_stmt(plc_codegen_context *c, stmt *s);
pl_bytecode_builder *plc_codegen_expr(plc_codegen_context *c, expr *e);

void plc_codegen_context_free(plc_codegen_context *c);

#endif