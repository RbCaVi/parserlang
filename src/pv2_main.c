#include "pv.h"
#include "pv_number.h"
#include "pv_singletons.h"
#include "pv_string.h"
#include "pv_array.h"
#include "pv_object.h"
#include "pv_to_string.h"

#include <stdio.h>

int main(int argc, char **argv) {
	pv_number_install();
	pv_singletons_install();
	pv_string_install();
	pv_array_install();
	pv_object_install();
	{
		pv val = pv_number(15);
		char *s = pv_to_string(val);
		double num = pv_number_value(val);
		printf("%f, %s\n", num, s);
	}
	{
		pv val = pv_true();
		char *s = pv_to_string(val);
		printf("%s\n", s);
	}
	{
		pv val = pv_false();
		char *s = pv_to_string(val);
		printf("%s\n", s);
	}
	{
		pv val = pv_null();
		char *s = pv_to_string(val);
		printf("%s\n", s);
	}
	{
		pv val = pv_string("cheesy");
		char *s = pv_to_string(val);
		printf("%s\n", s);
	}
	{
		pv val1 = pv_string("cheesy");
		pv val2 = pv_string("burger");
		char *s = pv_to_string(pv_string_concat(val1, val2));
		printf("%s\n", s);
	}
	{
		pv val = pv_array();
		char *s = pv_to_string(val);
		printf("%s\n", s);
	}
	{
		pv val = PV_ARRAY(pv_string("cheesy"));
		char *s = pv_to_string(val);
		printf("%s\n", s);
	}
	{
		pv val = PV_ARRAY(pv_string("cheesy"), pv_string("burger"));
		char *s = pv_to_string(val);
		printf("%s\n", s);
	}
	{
		pv val1 = PV_ARRAY(pv_string("cheesy"));
		pv val2 = PV_ARRAY(pv_string("burger"));
		char *s = pv_to_string(pv_array_concat(val1, val2));
		printf("%s\n", s);
	}
	{
		pv val1 = PV_ARRAY(pv_string("cheesy"));
		pv val2 = PV_ARRAY(pv_string("burger"));
		pv val = pv_array_concat(pv_copy(val1), val2);
		val = pv_array_concat(val, pv_copy(val1));
		val = pv_array_set(val, 1, val1);
		char *s = pv_to_string(val);
		printf("%s\n", s);
	}
	printf("%s\n", pv_kind_name(0));
}