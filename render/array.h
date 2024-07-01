#pragma once

#include <stddef.h>

typedef struct {
  size_t size;
  void *data;
} Array;

int array_init(Array *this, size_t size, void *data);
