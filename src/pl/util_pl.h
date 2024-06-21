#ifndef UTIL_PL_H
#define UTIL_PL_H

#include <stdlib.h>

// save the top in idx
// increments the top
// if top is out of bounds, reallocate for newsize elements
// initial is the size of anything before the array data
// idx is the name of a variable to store the index of the new element
// array is the array to add an element to
// top is the number of elements in the array
// size is the number of allocated slots
// assumes top, size, and array can be assigned to
#define inc_size(idx,array,top,initial,size,newsize) \
(idx) = (top); \
(top)++; \
if ((top) >= (size)) { \
  (size) = (newsize); \
  (array) = checked_realloc((array),(initial) + (size) * sizeof(typeof(*(array)))); \
}

void *checked_malloc(size_t);
void *checked_realloc(void*, size_t);

#endif