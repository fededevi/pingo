#include "target.h"

#include "state.h"

int render_target_init(RenderTarget *this, Vec2i size, Pixel *color,
                       PingoDepth *depth) {
  IF_NULL_RETURN(this, INIT_ERROR);

  const int e = texture_init(&this->color, size, color);
  if (e != OK) {
    return e;
  }

  // Allowed to be absent: a sprite-only target needs no depth.
  this->depth = depth;
  return OK;
}
