#pragma once

/**
 * Golden-image comparison, shared by the renderer's and the assets' tests.
 *
 * Memory budget: the reference is streamed one row at a time rather than held
 * as a second image, so the whole harness needs
 *
 *     width * height * (sizeof(Pixel) + sizeof(PingoDepth))   the buffers
 *   + width * 3 * 2                                           two RGB rows
 *
 * which at 64x48 with 4-byte pixels and a 32-bit depth buffer is about 24 KB -
 * inside the 64 KB a small microcontroller has, and inside 32 KB too. Holding
 * both images instead cost 42 KB.
 */

#include "render/pixel.h"

#include <stdio.h>

// Widest row the static row buffers can take.
#define GOLDEN_MAX_WIDTH 256

typedef enum {
  GOLDEN_OK = 0,
  GOLDEN_MISMATCH,  // rendered, compared, and too many pixels differ
  GOLDEN_BLANK,     // nothing drawn, so the comparison would prove nothing
  GOLDEN_IO_ERROR,  // the reference is missing, truncated or unwritable
} GoldenResult;

/**
 * Compares a rendered framebuffer against the PPM at `path`.
 *
 * Row 0 of `frame` is the bottom of the image; the PPM is written top-down so
 * that it opens the right way up in a viewer.
 *
 * The comparison is deliberately not exact: coverage is decided by floating
 * point, so a last-bit difference between architectures can flip a pixel on a
 * triangle edge from covered to uncovered - a full-value change. A few such
 * pixels are tolerated; a real regression moves far more.
 */
GoldenResult golden_compare(const char *path, const Pixel *frame, int width,
                            int height, int require_content);

/** Writes `frame` to `path` as a binary PPM, for --write-references. */
GoldenResult golden_write(const char *path, const Pixel *frame, int width,
                          int height);

/** Writes the rendered image beside the executable for eyeballing a failure. */
void golden_write_actual(const char *scene, const Pixel *frame, int width,
                         int height);
