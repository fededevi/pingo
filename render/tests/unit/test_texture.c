#include "render/texture.h"
#include "render/state.h"
#include "test_render_unit.h"

int test_texture(void) {
  printf("Testing Texture...\n");

  Pixel buf[4 * 4];
  Texture t;

  TEST_ASSERT(texture_init(&t, (Vec2i){4, 4}, buf) == OK, "valid init");
  TEST_ASSERT_EQ_INT(4, t.size.x, "width kept");
  TEST_ASSERT(t.pixels == buf, "buffer kept, not copied");

  // Rejections, so a caller cannot end up with a surface that has no memory.
  Texture bad;
  TEST_ASSERT(texture_init(&bad, (Vec2i){0, 4}, buf) != OK, "zero width");
  TEST_ASSERT(texture_init(&bad, (Vec2i){4, 0}, buf) != OK, "zero height");
  TEST_ASSERT(texture_init(&bad, (Vec2i){4, 4}, NULL) != OK, "null buffer");

  texture_fill(&t, pixel_from_uint8(7));
  for (int i = 0; i < 16; i++) {
    TEST_ASSERT_EQ_INT(7, pixel_to_uint8(&t.pixels[i]), "fill covers all");
  }

  texture_draw(&t, (Vec2i){2, 1}, pixel_from_uint8(9));
  TEST_ASSERT_EQ_INT(9, pixel_to_uint8(&t.pixels[1 * 4 + 2]), "draw at x,y");
  Pixel read_back = texture_read(&t, (Vec2i){2, 1});
  TEST_ASSERT_EQ_INT(9, pixel_to_uint8(&read_back), "read sees the write");

  // texture_draw_index must address the same pixel as texture_draw, since the
  // rasterizer uses the index form with an index it computed itself.
  texture_draw_index(&t, 1 * 4 + 3, pixel_from_uint8(11));
  Pixel by_index = texture_read(&t, (Vec2i){3, 1});
  TEST_ASSERT_EQ_INT(11, pixel_to_uint8(&by_index),
                     "index form agrees with x,y form");

  // UV sampling. The masked path is only valid for power-of-two sizes, and
  // both paths must agree wherever both are defined - including for negative
  // coordinates, which the interpolator produces just outside a triangle.
  TEST_ASSERT(texture_is_pow2(&t), "4x4 is power of two");
  Pixel np[3 * 4];
  Texture npot;
  texture_init(&npot, (Vec2i){3, 4}, np);
  TEST_ASSERT(!texture_is_pow2(&npot), "3x4 is not power of two");

  for (int i = 0; i < 16; i++) {
    t.pixels[i] = pixel_from_uint8((uint8_t)(i * 16));
  }
  const float uvs[] = {0.0f, 0.25f, 0.5f, 0.99f, -0.25f, -0.75f, 1.25f};
  for (unsigned i = 0; i < sizeof(uvs) / sizeof(uvs[0]); i++) {
    for (unsigned j = 0; j < sizeof(uvs) / sizeof(uvs[0]); j++) {
      const Vec2f uv = {uvs[i], uvs[j]};
      const Pixel general = texture_read_uv(&t, uv);
      const Pixel masked = texture_read_uv_pow2(&t, uv, 3, 3);
      if (memcmp(&general, &masked, sizeof(Pixel)) != 0) {
        printf("FAIL: masked and general sampling disagree at (%f, %f) at "
               "%s:%d\n",
               (double)uv.x, (double)uv.y, __FILE__, __LINE__);
        return 0;
      }
    }
  }

  return 1;
}
