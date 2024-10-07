#include "pv_install.h"
#include "pv_singletons.h"
#include "pv_number.h"
#include "pv_string.h"
#include "pv_array.h"
#include "pv_object.h"

void pv_install() {
	pv_singletons_install();
	pv_number_install();
	pv_string_install();
	pv_array_install();
	pv_object_install();
}