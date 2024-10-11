#include "pv.h"
#include "pv_number.h"
#include "pv_singletons.h"
#include "pv_string.h"
#include "pv_array.h"
#include "pv_object.h"
#include "pv_to_string.h"
#include "pv_install.h"

#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv) {
	(void)argc, (void)argv;
	pv_install();
	{
		pv val = pv_number(15);
		char *s = pv_to_string(val);
		double num = pv_number_value(val);
		printf("%f, %s\n", num, s);
		free(s);
	}
	printf("\n");
	{
		pv val = pv_true();
		char *s = pv_to_string(val);
		printf("%s\n", s);
		free(s);
	}
	printf("\n");
	{
		pv val = pv_false();
		char *s = pv_to_string(val);
		printf("%s\n", s);
		free(s);
	}
	printf("\n");
	{
		pv val = pv_null();
		char *s = pv_to_string(val);
		printf("%s\n", s);
		free(s);
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
		//printf("%i\n", pv_get_refcount(val));
		printf("%s\n", s);
		free(s);
		// test that the string is overwritten with AAAAAAA when freed
		// edit pv_string_free() as well to test (debugging thing)
		//char *s2 = pv_to_string(val);
		//printf("%i\n", pv_get_refcount(val));
		//pv_copy(val); // without this, segmentation fault???? (write to freed memory) (this resets it)
		//printf("%s\n", s2);
		printf("tostringd a string\n");
	}
	printf("\n");
	{
		printf("string concat\n");
		pv val1 = pv_string("cheesy");
		pv val2 = pv_string("burger");
		pv val3 = pv_string_concat(val1, val2);
		char *s = pv_to_string(val3);
		printf("%s\n", s);
		free(s);
	}
	printf("\n");
	{
		printf("array constructor\n");
		pv val = pv_array();
		char *s = pv_to_string(val);
		printf("%s\n", s);
		free(s);
	}
	printf("\n");
	{
		printf("array append\n");
		pv val1 = pv_string("cheesy");
		pv val = pv_array();
		val = pv_array_append(val, val1);
		char *s = pv_to_string(val);
		printf("%s\n", s);
		free(s);
	}
	printf("\n");
	{
		printf("array append (sized)\n");
		pv val1 = pv_string("cheesy");
		pv val = pv_array_sized(1);
		val = pv_array_append(val, val1);
		char *s = pv_to_string(val);
		printf("%s\n", s);
		free(s);
	}
	printf("\n");
	{
		printf("array constructor macro\n");
		pv val = PV_ARRAY(pv_string("cheesy"));
		char *s = pv_to_string(val);
		printf("%s\n", s);
		free(s);
	}
	printf("\n");
	{
		printf("array constructor macro 2\n");
		pv val = PV_ARRAY(pv_string("cheesy"), pv_string("burger"));
		char *s = pv_to_string(val);
		printf("%s\n", s);
		free(s);
	}
	printf("\n");
	{
		printf("array concatenate\n");
		pv val1 = PV_ARRAY(pv_string("cheesy"));
		pv val2 = PV_ARRAY(pv_string("burger"));
		char *s = pv_to_string(pv_array_concat(val1, val2));
		printf("%s\n", s);
		free(s);
	}
	printf("\n");
	{
		printf("array set\n");
		pv val1 = PV_ARRAY(pv_string("cheesy"));
		pv val2 = PV_ARRAY(pv_string("burger"));
		pv val = pv_array_concat(pv_copy(val1), val2);
		val = pv_array_concat(val, pv_copy(val1));
		val = pv_array_set(val, 1, val1);
		char *s = pv_to_string(val);
		printf("%s\n", s);
		free(s);
	}
	printf("\n");
	{
		printf("object constructor\n");
		pv val = pv_object();
		char *s = pv_to_string(val);
		printf("%s\n", s);
		free(s);
	}
	printf("\n");
	{
		pv val1 = pv_string("cheesy");
		pv val = pv_object();
		char *s1 = pv_to_string(pv_object_get(pv_copy(val), val1));
		char *s2 = pv_to_string(val);
		printf("%s %s\n", s1, s2);
		free(s1);
		free(s2);
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
		free(s1);
		free(s2);
	}
	printf("\n");
	{
		printf("object constructor macro\n");
		pv val = PV_OBJECT(pv_string("cheesy"), pv_string("burger"));
		char *s = pv_to_string(val);
		printf("%s\n", s);
		free(s);
	}
	printf("\n");
	{
		printf("object iterator\n");
		pv val = PV_OBJECT(pv_string("cheesy"), pv_string("burger"));
		int iter = pv_object_iter(pv_copy(val));
		char *s1 = pv_to_string(pv_object_iter_key(pv_copy(val), iter));
		char *s2 = pv_to_string(pv_object_iter_value(pv_copy(val), iter));
		printf("%i %s %s\n", iter, s1, s2);
		free(s1);
		free(s2);
		iter = pv_object_iter_next(pv_copy(val), iter);
		printf("%i %i\n", iter, pv_object_iter_valid(pv_copy(val), iter));
		// int pv_object_iter(pv);
		// int pv_object_iter_next(pv, int);
		// int pv_object_iter_valid(pv, int);
		// pv pv_object_iter_key(pv, int);
		// pv pv_object_iter_value(pv, int);
		char *s = pv_to_string(val);
		printf("%s\n", s);
		free(s);
	}
	printf("\n");
	{
		printf("object constructor macro - 2 keys\n");
		pv val = PV_OBJECT(pv_string("ask"), pv_string("hams"), pv_string("cheesy"), pv_string("burger"));
		printf("%i\n", pv_get_refcount(val));
		char *s = pv_to_string(val);
		printf("%s\n", s);
		free(s);
	}
	printf("%s\n", pv_kind_name(0));
}