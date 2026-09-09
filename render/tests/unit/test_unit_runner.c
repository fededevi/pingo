#include "test_render_unit.h"

typedef struct {
  const char *name;
  int (*func)(void);
} UnitCase;

static const UnitCase cases[] = {
    {"pixel", test_pixel},
    {"texture", test_texture},
    {"depth", test_depth},
    {"target", test_target},
    {"transform", test_transform},
    {"drawables", test_object_sprite_material},
    {"renderer", test_renderer},
};

static const size_t case_count = sizeof(cases) / sizeof(cases[0]);

static int run_case(const UnitCase *c) {
  printf("Running %s...\n", c->name);
  if (c->func()) {
    printf("PASS: %s\n\n", c->name);
    return 1;
  }
  printf("FAIL: %s\n\n", c->name);
  return 0;
}

int run_all_unit_tests(void) {
  int ok = 1;
  for (size_t i = 0; i < case_count; i++) {
    if (!run_case(&cases[i])) {
      ok = 0;
    }
  }
  return ok;
}

int run_unit_test_by_name(const char *name) {
  for (size_t i = 0; i < case_count; i++) {
    if (strcmp(cases[i].name, name) == 0) {
      return run_case(&cases[i]);
    }
  }
  return -1;
}

void print_unit_test_names(FILE *out) {
  for (size_t i = 0; i < case_count; i++) {
    fprintf(out, "%s%s", i ? " " : "", cases[i].name);
  }
}
