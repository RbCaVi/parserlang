#ifndef PV_STRING_H
#define PV_STRING_H

pv pv_string(const char*);
pv pv_string_sized(const char*, int);
pv pv_string_empty(int len);
int pv_string_length_bytes(pv);
int pv_string_length_codepoints(pv);
unsigned long pv_string_hash(pv);
const char* pv_string_value(pv);
pv pv_string_indexes(pv j, pv k);
pv pv_string_slice(pv j, int start, int end);
pv pv_string_concat(pv, pv);
pv pv_string_vfmt(const char*, va_list) PV_VPRINTF_LIKE(1);
pv pv_string_fmt(const char*, ...) PV_PRINTF_LIKE(1, 2);
pv pv_string_append_codepoint(pv a, uint32_t c);
pv pv_string_append_buf(pv a, const char* buf, int len);
pv pv_string_append_str(pv a, const char* str);
pv pv_string_split(pv j, pv sep);
pv pv_string_explode(pv j);
pv pv_string_implode(pv j);

#endif