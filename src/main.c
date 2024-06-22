#include "pv/pv.h"
#include "pv/pv_array.h"
#include "pv/pv_number.h"
#include "pv/pv_constants.h"
#include "pv/pv_object.h"
#include "pv/pv_string.h"
#include "pl/pl_dump.h"
#include "pl/pl_stack.h"

int main(int, char**) {
	pv n = PV_ARRAY(pv_number(15),pv_false(),PV_OBJECT(pv_string("key"),pv_string("value")));
	pl_dump_pv(n);
	return 0;
}
