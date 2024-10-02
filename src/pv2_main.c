#include "pv.h"
#include "pv_number.h"
#include "pv_singletons.h"
#include "pv_string.h"
#include "pv_array.h"
#include "pv_object.h"
#include "pv_to_string.h"

#include <stdio.h>

int main(int argc, char **argv) {
	(void)argc, (void)argv;
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
	printf("\n");
	{
		pv val = pv_true();
		char *s = pv_to_string(val);
		printf("%s\n", s);
	}
	printf("\n");
	{
		pv val = pv_false();
		char *s = pv_to_string(val);
		printf("%s\n", s);
	}
	printf("\n");
	{
		pv val = pv_null();
		char *s = pv_to_string(val);
		printf("%s\n", s);
	}
	printf("\n");
	{
		printf("freeing a string\n");
		pv val = pv_string("cheesy");
		pv_free(val);
		printf("freed a string\n");
	}
	printf("\n");
	{
		printf("tostring a string\n");
		pv val = pv_string("cheesy");
		printf("%i\n", pv_get_refcount(val));
		char *s = pv_to_string(val);
		printf("%i\n", pv_get_refcount(val));
		printf("%s\n", s);
		printf("tostringd a string\n");
	}
	printf("\n");
	{
		pv val1 = pv_string("cheesy");
		pv val2 = pv_string("burger");
		char *s = pv_to_string(pv_string_concat(val1, val2));
		printf("%s\n", s);
	}
	printf("\n");
	{
		pv val = pv_array();
		char *s = pv_to_string(val);
		printf("%s\n", s);
	}
	printf("\n");
	{
		pv val = PV_ARRAY(pv_string("cheesy"));
		char *s = pv_to_string(val);
		printf("%s\n", s);
	}
	printf("\n");
	{
		pv val = PV_ARRAY(pv_string("cheesy"), pv_string("burger"));
		char *s = pv_to_string(val);
		printf("%s\n", s);
	}
	printf("\n");
	{
		pv val1 = PV_ARRAY(pv_string("cheesy"));
		pv val2 = PV_ARRAY(pv_string("burger"));
		char *s = pv_to_string(pv_array_concat(val1, val2));
		printf("%s\n", s);
	}
	printf("\n");
	{
		pv val1 = PV_ARRAY(pv_string("cheesy"));
		pv val2 = PV_ARRAY(pv_string("burger"));
		pv val = pv_array_concat(pv_copy(val1), val2);
		val = pv_array_concat(val, pv_copy(val1));
		val = pv_array_set(val, 1, val1);
		char *s = pv_to_string(val);
		printf("%s\n", s);
	}
	printf("\n");
	{
		pv val = pv_object();
		char *s = pv_to_string(val);
		printf("%s\n", s);
	}
	printf("\n");
	{
		pv val1 = pv_string("cheesy");
		pv val = pv_object();
		char *s1 = pv_to_string(pv_object_get(pv_copy(val), val1));
		char *s2 = pv_to_string(val);
		printf("%s %s\n", s1, s2);
	}
	printf("\n");
	{
		pv val1 = pv_string("cheesy");
		pv val2 = pv_string("burger");
		pv val3 = pv_object();
		val3 = pv_object_set(val3, pv_copy(val1), val2);
		char *s1 = pv_to_string(pv_object_get(pv_copy(val3), val1));
		char *s2 = pv_to_string(val3);
		printf("%s %s\n", s1, s2);
	}
	printf("%s\n", pv_kind_name(0));
}