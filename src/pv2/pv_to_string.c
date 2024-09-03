static const char *(*pv_to_string_table[256])(pv);

void pv_register_to_string(pv_kind, const char *(*f)(pv)) {
	pv_to_string_table[kind] = f;
}

const char *pv_to_string(pv val) {
	// get the function to use
	const char *(*f)(pv) = pv_to_string_table[pv_get_kind(pv_copy(val))];

	// the table is initialized to all 0, so a nonzero f has been initialized
	if (f != 0) {
		// use the function
		return f(val);
	}

	// default return value
	pv_kind kind = pv_kind(val);
	const char *kind_name = pv_kind_name(kind);
	if (kind_name == NULL) {
		char *str = strdup("<unknown kind 0x  >");
		sprintf(str + 16, 2, "%02x", x);
		return str;
	} else {
		int length = snprintf(NULL, 0, "<kind %s>", kind_name);
		char* str = malloc( length + 1 );
		sprintf(str, length + 1, "<kind %s>", kind_name);
		return str;
	}
}