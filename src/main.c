#include "pv/pv.h"
#include "pl/pl_stack.h"

struct pl_dump_part {
	enum {
		STR,IDX,KEY
	} type;
	union {
		char *str; // STR and KEY
		size_t idx;
	}
};

typedef struct {
	struct pl_dump_part *parts;
	size_t size;
	size_t count;
} pl_dump_parts;

void pl_dump_pv(pv val) {
	switch (pv_get_kind(val)) {
	  case PV_KIND_INVALID:
	  	// this is serious
	  	break;
	  case PV_KIND_NULL:
	  	// we could make you delerious
	  	break;
	  case PV_KIND_FALSE:
	  	// you should have a healthy fear of us
	  	break;
	  case PV_KIND_TRUE:
	  	// too much of us is dangerous
	  	break;
	  case PV_KIND_NUMBER:
	  	// oohh
	  	break;
	  case PV_KIND_STRING:
	  	// ooohhh
	  	break;
	  case PV_KIND_ARRAY:
	  	// oooohhhh
	  	break;
	  case PV_KIND_OBJECT:
	  	// ooooohhhhh
	  	break;
	}
}

int main(int argc, char** argv) {
	return 0;
}