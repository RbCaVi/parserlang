#include "pv.h"
#include "pv_number.h"
#include "pv_to_string.h"

#include <stdio.h>

int main(int argc, char **argv) {
	pv val = pv_number(15);
	char *s = pv_to_string(val);
	double num = pv_number_value(val);
	printf("%f, %s\n", num, s);
	printf("%s\n", pv_kind_name(0));
}