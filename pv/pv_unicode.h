#ifndef JV_UNICODE_H
#define JV_UNICODE_H

const char* pvp_utf8_backtrack(const char* start, const char* min, int *missing_bytes);
const char* pvp_utf8_next(const char* in, const char* end, int* codepoint);
int pvp_utf8_is_valid(const char* in, const char* end);

int pvp_utf8_decode_length(char startchar);

int pvp_utf8_encode_length(int codepoint);
int pvp_utf8_encode(int codepoint, char* out);
#endif
