#include "test_render_unit.h"

// Same contract as math_tests: no argument runs every suite, a name runs one,
// which is how CTest registers them individually.
int main(int argc, char **argv) {
  if (argc > 2) {
    fprintf(stderr, "usage: %s [suite]\n", argv[0]);
    fprintf(stderr, "suites: ");
    print_unit_test_names(stderr);
    fprintf(stderr, "\n");
    return 2;
  }

  if (argc == 2) {
    const int result = run_unit_test_by_name(argv[1]);
    if (result < 0) {
      fprintf(stderr, "error: no test suite named '%s'\n", argv[1]);
      fprintf(stderr, "suites: ");
      print_unit_test_names(stderr);
      fprintf(stderr, "\n");
      return 2;
    }
    return result ? 0 : 1;
  }

  printf("=== Pingo Render Unit Tests ===\n\n");
  if (run_all_unit_tests()) {
    printf("\nAll unit tests passed.\n");
    return 0;
  }
  printf("\nSome unit tests failed.\n");
  return 1;
}
