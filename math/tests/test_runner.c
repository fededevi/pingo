#include "test_math.h"

#include <string.h>

// The suites, in one table so that running all of them and running one by name
// share a single source of truth. The names double as the CTest test names.
typedef struct {
  const char *name;
  int (*func)(void);
} TestCase;

static const TestCase test_cases[] = {
    {"vec2", test_vec2}, {"vec3", test_vec3}, {"vec4", test_vec4},
    {"mat3", test_mat3}, {"mat4", test_mat4}, {"functions", test_functions},
};

static const size_t test_case_count =
    sizeof(test_cases) / sizeof(test_cases[0]);

static int run_case(const TestCase *test_case) {
  printf("Running %s...\n", test_case->name);
  if (test_case->func()) {
    printf("PASS: %s\n\n", test_case->name);
    return 1;
  }
  printf("FAIL: %s\n\n", test_case->name);
  return 0;
}

int run_all_tests(void) {
  printf("Running math library tests...\n\n");

  size_t passed = 0;
  for (size_t i = 0; i < test_case_count; i++) {
    passed += (size_t)run_case(&test_cases[i]);
  }

  printf("Test Results: %zu/%zu tests passed\n", passed, test_case_count);
  return passed == test_case_count;
}

int run_test_by_name(const char *name) {
  for (size_t i = 0; i < test_case_count; i++) {
    if (strcmp(test_cases[i].name, name) == 0) {
      return run_case(&test_cases[i]);
    }
  }
  return -1;
}

void print_test_names(FILE *out) {
  for (size_t i = 0; i < test_case_count; i++) {
    fprintf(out, "%s%s", i ? " " : "", test_cases[i].name);
  }
}
