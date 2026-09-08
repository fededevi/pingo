#include "test_math.h"

// With no argument every suite runs, which is what `./tests/math_tests` has
// always done. With a suite name only that one runs, which is how CTest
// registers them individually so a failure names the suite that broke.
int main(int argc, char **argv) {
  if (argc > 2) {
    fprintf(stderr, "usage: %s [suite]\n", argv[0]);
    fprintf(stderr, "suites: ");
    print_test_names(stderr);
    fprintf(stderr, "\n");
    return 2;
  }

  if (argc == 2) {
    int result = run_test_by_name(argv[1]);
    if (result < 0) {
      fprintf(stderr, "error: no test suite named '%s'\n", argv[1]);
      fprintf(stderr, "suites: ");
      print_test_names(stderr);
      fprintf(stderr, "\n");
      return 2;
    }
    return result ? 0 : 1;
  }

  printf("=== Pingo Math Library Test Suite ===\n\n");

  if (run_all_tests()) {
    printf("\n🎉 All tests passed!\n");
    return 0;
  }

  printf("\n❌ Some tests failed!\n");
  return 1;
}
