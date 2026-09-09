#pragma once

#include <stddef.h>

/**
 * A count and a pointer. `count` is elements, not bytes - the caller knows
 * the element type, this does not.
 */
typedef struct {
  size_t count;
  void *data;
} Array;

int array_init(Array *this, size_t count, void *data);
