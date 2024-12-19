#include "pv_userdata.h"
#include "pv_to_string.h"
#include "pv_equal.h"

#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

pv_kind userdata_kind;

static char *pv_userdata_to_string(pv val) {
	void *ptr = pv_userdata_ptr(val);
	int l = snprintf(NULL, 0, "%p", ptr);
	assert(l >= 0);
	size_t length = (size_t)l;
	char* str = malloc(length + 1);
	snprintf(str, length + 1, "%p", ptr);
	return str;
}

static int pv_userdata_equal_self(pv val1, pv val2) {
	return pv_userdata_ptr(val1) == pv_userdata_ptr(val2);
}

void pv_userdata_install() {
	pv_register_kind(&userdata_kind, "userdata", NULL);
	pv_register_to_string(userdata_kind, pv_userdata_to_string);
	pv_register_equal_self(userdata_kind, pv_userdata_equal_self);
}

pv pv_userdata(void *ptr) {
	pv val = {userdata_kind, 0, ptr};
	return val;
}

void *pv_userdata_ptr(pv val) {
	// don't have to do a decref because userdata isn't allocated
	assert(val.kind == userdata_kind);
	void *ptr = val.data;
	return ptr;
}