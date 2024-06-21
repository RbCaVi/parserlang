#include "pl/util_pl.h"

void *checked_malloc(size_t size) {
  void *p = malloc(size);
  if (p == NULL) {
    abort();
  }
  return p;
}

void *checked_realloc(void *p, size_t size) {
  p = realloc(p, size);
  if (p == NULL) {
    abort();
  }
  return p;
}