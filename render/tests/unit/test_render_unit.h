#pragma once

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Same shape as math/tests/test_math.h: a suite returns 1 for pass and 0 for
// fail, and each assertion reports where it broke before returning.
#define TEST_ASSERT(condition, message)                                        \
  do {                                                                         \
    if (!(condition)) {                                                        \
      printf("FAIL: %s at %s:%d\n", message, __FILE__, __LINE__);              \
      return 0;                                                                \
    }                                                                          \
  } while (0)

#define TEST_ASSERT_EQ_INT(expected, actual, message)                          \
  do {                                                                         \
    const long e_ = (long)(expected), a_ = (long)(actual);                     \
    if (e_ != a_) {                                                            \
      printf("FAIL: %s: expected %ld, got %ld at %s:%d\n", message, e_, a_,    \
             __FILE__, __LINE__);                                              \
      return 0;                                                                \
    }                                                                          \
  } while (0)

int test_pixel(void);
int test_texture(void);
int test_depth(void);
int test_target(void);
int test_transform(void);
int test_object_sprite_material(void);
int test_renderer(void);
int test_span(void);

int run_all_unit_tests(void);
int run_unit_test_by_name(const char *name);
void print_unit_test_names(FILE *out);
