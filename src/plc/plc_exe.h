#ifndef PLC_EXE_H
#define PLC_EXE_H

#include "pv.h"

typedef struct pl_exe pl_exe;

pl_exe *pl_exe_new();

unsigned int pl_exe_add_global(pl_exe *data, pv val);

void pl_exe_set_main(pl_exe *data, pv val);

void pl_exe_dump_file(pl_exe *exe, char *file);

void pl_exe_free(pl_exe *exe);

#endif