#include "render/depth.h"
#include "test_render_unit.h"

int test_depth(void) {
  printf("Testing depth buffer...\n");

  PingoDepth d[4];
  memset(d, 0, sizeof(d));

  // The convention is the one depth_check always used: a stored value that is
  // greater wins, so a cleared buffer of zeros accepts anything.
  TEST_ASSERT(depth_test_and_write(d, 0, 0.5f), "first write into cleared");
  const PingoDepthValue half = d[0].d;
  TEST_ASSERT(half != 0, "value was stored");

  TEST_ASSERT(!depth_test_and_write(d, 0, 0.25f), "smaller is rejected");
  TEST_ASSERT_EQ_INT(half, d[0].d, "rejected write left the buffer alone");

  TEST_ASSERT(depth_test_and_write(d, 0, 0.75f), "larger is accepted");
  TEST_ASSERT(d[0].d > half, "accepted write updated the buffer");

  // Equal is accepted, not rejected. depth_check has always been
  // `new < stored` for the reject, so an equal value falls through and draws.
  // The consequence is worth knowing: coplanar surfaces are decided by draw
  // order, with the last one submitted winning. Asserted so that changing it
  // is a deliberate decision rather than a side effect.
  const PingoDepthValue kept = d[0].d;
  TEST_ASSERT(depth_test_and_write(d, 0, 0.75f), "equal is accepted");
  TEST_ASSERT_EQ_INT(kept, d[0].d, "equal stores the same value");

  // Neighbouring indices must be independent.
  TEST_ASSERT(depth_test_and_write(d, 1, 0.1f), "index 1 is its own slot");
  TEST_ASSERT_EQ_INT(kept, d[0].d, "index 1 did not touch index 0");

  // The extremes of the range the rasterizer permits.
  TEST_ASSERT(depth_test_and_write(d, 2, 1.0f), "depth 1 accepted");
  // Depth 0 against a zeroed slot is the equal case again, so it draws.
  memset(&d[3], 0, sizeof(d[3]));
  TEST_ASSERT(depth_test_and_write(d, 3, 0.0f), "depth 0 into zeroed slot");

  return 1;
}
