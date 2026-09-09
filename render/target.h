#pragma once

#include "fwd.h"
#include "texture.h"

/**
 * Where the renderer draws: a colour surface and the depth buffer that goes
 * with it.
 *
 * The two used to be reached through separate Backend calls returning bare
 * pointers, which left the depth buffer with no stated size - its accessor's
 * comment even described it in units of Pixel. Here the colour surface's size
 * governs both, so a backend cannot hand over a depth buffer that disagrees
 * with the framebuffer it belongs to.
 *
 * A Texture by composition rather than by extension: a sprite blit and a
 * source image are Textures and have no use for depth, so depth does not
 * belong in Texture itself.
 */
struct RenderTarget {
  Texture color;

  /** One entry per pixel of `color`. NULL for a target drawn without depth. */
  PingoDepth *depth;
};

/** Fails if either buffer is NULL or the size is empty. */
extern int render_target_init(RenderTarget *this, Vec2i size, Pixel *color,
                              PingoDepth *depth);
