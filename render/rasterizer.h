#pragma once

#include "renderer.h"
#include "sprite.h"
#include "texture.h"

/**
 * Defines the type of filtering used when textures are resized and rotated
 * by a transformation. Nearest filtering just take 1 texture sample in the
 * single pixel which the source coordinate is tranformed to.
 * Bilinear filtering takes 4 samples on the source texture to compute a
 * weighted average of those 4 values based on the distance of the input
 * point.
 * Anisotropic filtering takes 4/16 samples from the source image
 * distributed uniformly over the area occupied by the destination pixel on
 * the source image and makes an average of those values.
 */

/*
 * NEAREST is the one that is finished. Of the others:
 *
 *  - BILINEAR does not build: it calls a frameReadBilinear that was never
 *    written.
 *  - ANISOTROPIC and ANISOTROPICX2 build, but average a single channel and
 *    construct their Pixel from one initializer, so they assume
 *    PINGO_PIXEL_UINT8 and drop colour silently in any other format.
 *
 * Only one may be defined at a time: the blocks in rasterizer.c declare the
 * same locals.
 */
#define FILTERING_NEAREST
// #define FILTERING_BILINEAR    // unfinished, see above
// #define FILTERING_ANISOTROPIC // single-channel only, see above
// #define FILTERING_ANISOTROPICX2

int rasterizer_draw_pixel_perfect(Vec2i off, Renderer *r, Texture *src);

int rasterizer_draw_pixel_perfect_doubled(Vec2i off, Renderer *r, Texture *src);

int rasterizer_draw_transformed(Mat4 t, Renderer *r, Texture *src);
