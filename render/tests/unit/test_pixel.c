#include "render/pixel.h"
#include "test_render_unit.h"

int test_pixel(void) {
  printf("Testing Pixel...\n");

  const Pixel white = pixel_from_uint8(255);
  const Pixel black = pixel_from_uint8(0);
  TEST_ASSERT_EQ_INT(255, pixel_to_uint8((Pixel *)&white), "white round trip");
  TEST_ASSERT_EQ_INT(0, pixel_to_uint8((Pixel *)&black), "black round trip");

  // pixel_from_rgba is the only place the channel order of the configured
  // format is visible, so assert through pixel_to_uint8 rather than by field.
  const Pixel grey = pixel_from_rgba(128, 128, 128, 255);
  TEST_ASSERT_EQ_INT(128, pixel_to_uint8((Pixel *)&grey), "grey round trip");

  // Scaling by one and by zero are the two ends that must be exact.
  const Pixel unchanged = pixel_mul(white, 1.0f);
  TEST_ASSERT_EQ_INT(255, pixel_to_uint8((Pixel *)&unchanged), "scale by 1");
  const Pixel zeroed = pixel_mul(white, 0.0f);
  TEST_ASSERT_EQ_INT(0, pixel_to_uint8((Pixel *)&zeroed), "scale by 0");

  // The shade table exists to give the same answer as pixel_mul without the
  // per-pixel conversions. If it ever disagrees the renderer's output depends
  // on triangle size, so check every channel value against every factor.
  for (int f = 0; f <= 64; f++) {
    const float factor = (float)f / 64.0f;
    PixelShadeTable t;
    pixel_shade_table_init(&t, factor);
    for (int v = 0; v < 256; v++) {
      const Pixel src = pixel_from_uint8((uint8_t)v);
      const Pixel by_mul = pixel_mul(src, factor);
      const Pixel by_table = pixel_mul_table(src, &t);
      if (memcmp(&by_mul, &by_table, sizeof(Pixel)) != 0) {
        printf("FAIL: shade table disagrees with pixel_mul at value %d, "
               "factor %f at %s:%d\n",
               v, (double)factor, __FILE__, __LINE__);
        return 0;
      }
    }
  }

  return 1;
}
