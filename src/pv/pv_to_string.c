#include "pv_to_string.h"

#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

#ifndef strdup
char *strdup (const char *);
#endif

static pv_to_string_func pv_to_string_table[256];

void pv_register_to_string(pv_kind kind, pv_to_string_func f) {
	pv_to_string_table[kind] = f;
}

char *pv_to_string(pv val) {
	// get the function to use
	char *(*f)(pv) = pv_to_string_table[pv_get_kind(val)];

	// the table is initialized to all 0, so a nonzero f has been initialized
	if (f != 0) {
		// use the function
		return f(val);
	}

	// default return value
	pv_kind kind = pv_get_kind(val);
	pv_free(val); // not needed anymore
	if (kind == 0) {
		return strdup("<INVALID>");
	}
	const char *kind_name = pv_kind_name(kind);
	if (kind_name == NULL) {
		char *str = strdup("<value of unknown kind 0x   ");
		snprintf(str + 16, 4, "%.2x>", kind);
		return str;
	} else {
		// printf type functions can have a negative return if there is an error
		// how am i handling it?
		// uhhhhhhhhhhh
		// oh look a segmentation fault i have to fix that
		int l = snprintf(NULL, 0, "<value of kind %s>", kind_name);
		assert(l >= 0); // (death)
		size_t length = (size_t)l;
		char* str = malloc(length + 1);
		snprintf(str, length + 1, "<value of kind %s>", kind_name);
		return str;
	}
}