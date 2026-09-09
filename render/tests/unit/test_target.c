#include "render/target.h"
#include "render/depth.h"
#include "render/state.h"
#include "test_render_unit.h"

int test_target(void) {
  printf("Testing RenderTarget...\n");

  Pixel colour[8 * 8];
  PingoDepth depth[8 * 8];
  RenderTarget t;

  TEST_ASSERT(render_target_init(&t, (Vec2i){8, 8}, colour, depth) == OK,
              "valid init");
  TEST_ASSERT(t.color.pixels == colour, "colour buffer kept");
  TEST_ASSERT(t.depth == depth, "depth buffer kept");
  TEST_ASSERT_EQ_INT(8, t.color.size.x, "size reaches the colour surface");

  // The whole point of the type: one size governs both buffers, so a caller
  // cannot describe a depth buffer that disagrees with the framebuffer.
  TEST_ASSERT_EQ_INT(8 * 8, t.color.size.x * t.color.size.y,
                     "depth length is implied by the colour size");

  // Depth is optional - a sprite-only target has none - but colour is not.
  RenderTarget no_depth;
  TEST_ASSERT(render_target_init(&no_depth, (Vec2i){8, 8}, colour, NULL) == OK,
              "depth may be absent");
  TEST_ASSERT(no_depth.depth == NULL, "absent depth stays absent");

  RenderTarget bad;
  TEST_ASSERT(render_target_init(&bad, (Vec2i){0, 8}, colour, depth) != OK,
              "empty size rejected");
  TEST_ASSERT(render_target_init(&bad, (Vec2i){8, 8}, NULL, depth) != OK,
              "null colour rejected");
  TEST_ASSERT(render_target_init(NULL, (Vec2i){8, 8}, colour, depth) != OK,
              "null target rejected");

  return 1;
}
