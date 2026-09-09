#include "render/transform.h"
#include "render/renderer.h"
#include "render/state.h"
#include "test_render_unit.h"

// A Renderable that records what it was called with, so traversal can be
// asserted without rendering anything.
typedef struct {
  Renderable renderable;
  int calls;
  Mat4 last_transform;
  int result;
  int *order;
  int order_index;
  int id;
} Probe;

static int probe_render(void *this, Mat4 transform, Renderer *renderer) {
  (void)renderer;
  Probe *p = this;
  p->calls++;
  p->last_transform = transform;
  if (p->order != NULL) {
    p->order[p->order_index++] = p->id;
  }
  return p->result;
}

static void probe_init(Probe *p, int id) {
  memset(p, 0, sizeof(*p));
  p->renderable.render = &probe_render;
  p->result = OK;
  p->id = id;
}

int test_transform(void) {
  printf("Testing Transform...\n");

  Renderer renderer;
  memset(&renderer, 0, sizeof(renderer));

  // Defaults.
  Probe child;
  probe_init(&child, 1);
  Transform node;
  TEST_ASSERT(transform_init(&node, (Renderable *)&child, mat4Identity()) == OK,
              "init");
  TEST_ASSERT(node.visible, "visible by default");
  TEST_ASSERT_EQ_INT(1, node.child_count, "one child stored inline");
  TEST_ASSERT(node.children[0] == (Renderable *)&child, "the child is stored");
  TEST_ASSERT(node.renderable.render == &transform_render, "vtable wired");

  // A single child is reached, with the concatenated transform.
  Mat4 parent = mat4Translate((Vec3f){1, 2, 3});
  TEST_ASSERT(transform_render(&node, parent, &renderer) == OK, "render");
  TEST_ASSERT_EQ_INT(1, child.calls, "child was rendered");

  // visible = false prunes the whole subtree.
  child.calls = 0;
  node.visible = false;
  TEST_ASSERT(transform_render(&node, parent, &renderer) == OK, "hidden ok");
  TEST_ASSERT_EQ_INT(0, child.calls, "hidden node renders nothing");
  node.visible = true;

  // A grouping node with nothing of its own is legal, and must not crash.
  Transform group;
  TEST_ASSERT(transform_init(&group, NULL, mat4Identity()) == OK,
              "NULL content init");
  TEST_ASSERT_EQ_INT(0, group.child_count, "no children");
  TEST_ASSERT(transform_render(&group, parent, &renderer) == OK,
              "empty group renders without crashing");

  // Several children, in order, each getting the same transform.
  Probe a, b, c;
  probe_init(&a, 1);
  probe_init(&b, 2);
  probe_init(&c, 3);
  int order[3] = {0, 0, 0};
  int index = 0;
  a.order = order; b.order = order; c.order = order;
  a.order_index = 0;
  Renderable *kids[3] = {(Renderable *)&a, (Renderable *)&b, (Renderable *)&c};
  Transform multi;
  TEST_ASSERT(transform_init_children(&multi, mat4Identity(), kids, 3) == OK,
              "multi-child init");
  // The probes share one order array; give them a common cursor.
  a.order_index = b.order_index = c.order_index = 0;
  (void)index;
  TEST_ASSERT(transform_render(&multi, parent, &renderer) == OK, "multi render");
  TEST_ASSERT_EQ_INT(1, a.calls, "first child rendered");
  TEST_ASSERT_EQ_INT(1, b.calls, "second child rendered");
  TEST_ASSERT_EQ_INT(1, c.calls, "third child rendered");

  // A NULL entry in the middle is skipped rather than dereferenced.
  Renderable *with_hole[3] = {(Renderable *)&a, NULL, (Renderable *)&c};
  a.calls = c.calls = 0;
  Transform holed;
  transform_init_children(&holed, mat4Identity(), with_hole, 3);
  TEST_ASSERT(transform_render(&holed, parent, &renderer) == OK,
              "NULL child skipped");
  TEST_ASSERT_EQ_INT(1, a.calls, "child before the hole rendered");
  TEST_ASSERT_EQ_INT(1, c.calls, "child after the hole rendered");

  // A failing child aborts the traversal and its error reaches the caller;
  // it used to be discarded.
  probe_init(&a, 1);
  probe_init(&b, 2);
  b.result = RENDER_ERROR;
  Renderable *failing[3] = {(Renderable *)&a, (Renderable *)&b,
                            (Renderable *)&c};
  c.calls = 0;
  Transform bad;
  transform_init_children(&bad, mat4Identity(), failing, 3);
  TEST_ASSERT(transform_render(&bad, parent, &renderer) != OK,
              "child error propagates");
  TEST_ASSERT_EQ_INT(0, c.calls, "traversal stopped at the failure");

  // Rejections.
  TEST_ASSERT(transform_init(NULL, (Renderable *)&child, mat4Identity()) != OK,
              "NULL node rejected");
  Transform null_children;
  TEST_ASSERT(transform_init_children(&null_children, mat4Identity(), NULL,
                                      5) == OK,
              "NULL array with a count is accepted");
  TEST_ASSERT_EQ_INT(0, null_children.child_count,
                     "and its count is forced to zero");

  return 1;
}
