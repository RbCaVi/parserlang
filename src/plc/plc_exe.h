#ifndef PLC_EXE_H
#define PLC_EXE_H

#include "pv.h"

typedef struct {
	pv main;
	unsigned int glen;
	pv *globals;
} plc_exe;

plc_exe pl_exe_parse(char*);
void plc_exe_dump(plc_exe, char*);

#endif