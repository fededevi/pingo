#include "golden.h"

#include <stdlib.h>
#include <string.h>

// Per-channel difference treated as noise rather than a change.
#define CHANNEL_TOLERANCE 2
// Share of pixels allowed to differ beyond that, for edge-coverage flips.
#define MAX_DIFFERING_PERMILLE 5 // 0.5%
// Below this share of lit pixels the scene is not in view, and comparing it
// against an equally blank reference would prove nothing.
#define MIN_LIT_PERMILLE 10 // 1%

// Two rows, not two images: this is what keeps the harness inside 64 KB.
static unsigned char row_actual[GOLDEN_MAX_WIDTH * 3];
static unsigned char row_expected[GOLDEN_MAX_WIDTH * 3];

// Row 0 of the framebuffer is the bottom of the image, so rows are emitted in
// reverse and the PPM opens the right way up.
static void row_to_rgb(const Pixel *frame, int width, int height, int y,
                       unsigned char *rgb) {
  const Pixel *src = &frame[(height - 1 - y) * width];
  for (int x = 0; x < width; x++) {
#ifdef PINGO_PIXEL_UINT8
    rgb[x * 3 + 0] = src[x].g;
    rgb[x * 3 + 1] = src[x].g;
    rgb[x * 3 + 2] = src[x].g;
#else
    rgb[x * 3 + 0] = src[x].r;
    rgb[x * 3 + 1] = src[x].g;
    rgb[x * 3 + 2] = src[x].b;
#endif
  }
}

// Reads and validates the P6 header, leaving the stream on the pixel data.
static int open_reference(const char *path, int width, int height, FILE **out) {
  FILE *f = fopen(path, "rb");
  if (f == NULL) {
    fprintf(stderr, "  cannot open reference %s\n", path);
    fprintf(stderr, "  run this executable with --write-references first\n");
    return 1;
  }

  int w = 0, h = 0, maxval = 0;
  if (fscanf(f, "P6 %d %d %d", &w, &h, &maxval) != 3) {
    fprintf(stderr, "  %s is not a binary PPM\n", path);
    fclose(f);
    return 1;
  }
  if (w != width || h != height || maxval != 255) {
    fprintf(stderr, "  %s is %dx%d maxval %d, expected %dx%d maxval 255\n",
            path, w, h, maxval, width, height);
    fclose(f);
    return 1;
  }
  fgetc(f); // the single whitespace byte before the data

  *out = f;
  return 0;
}

GoldenResult golden_write(const char *path, const Pixel *frame, int width,
                          int height) {
  if (width > GOLDEN_MAX_WIDTH) {
    fprintf(stderr, "  width %d exceeds GOLDEN_MAX_WIDTH %d\n", width,
            GOLDEN_MAX_WIDTH);
    return GOLDEN_IO_ERROR;
  }

  FILE *f = fopen(path, "wb");
  if (f == NULL) {
    fprintf(stderr, "  cannot write %s\n", path);
    return GOLDEN_IO_ERROR;
  }

  fprintf(f, "P6\n%d %d\n255\n", width, height);
  for (int y = 0; y < height; y++) {
    row_to_rgb(frame, width, height, y, row_actual);
    if (fwrite(row_actual, 1, (size_t)width * 3, f) != (size_t)width * 3) {
      fprintf(stderr, "  short write to %s\n", path);
      fclose(f);
      return GOLDEN_IO_ERROR;
    }
  }

  if (fclose(f) != 0) {
    fprintf(stderr, "  failed to close %s\n", path);
    return GOLDEN_IO_ERROR;
  }
  return GOLDEN_OK;
}

void golden_write_actual(const char *scene, const Pixel *frame, int width,
                         int height) {
  char path[256];
  snprintf(path, sizeof(path), "%s-actual.ppm", scene);
  if (golden_write(path, frame, width, height) == GOLDEN_OK) {
    fprintf(stderr, "  wrote %s for comparison\n", path);
  }
}

GoldenResult golden_compare(const char *path, const Pixel *frame, int width,
                            int height, int require_content) {
  if (width > GOLDEN_MAX_WIDTH) {
    fprintf(stderr, "  width %d exceeds GOLDEN_MAX_WIDTH %d\n", width,
            GOLDEN_MAX_WIDTH);
    return GOLDEN_IO_ERROR;
  }

  FILE *f = NULL;
  if (open_reference(path, width, height, &f) != 0) {
    return GOLDEN_IO_ERROR;
  }

  const long pixels = (long)width * height;
  const int allowed = (int)(pixels * MAX_DIFFERING_PERMILLE / 1000);
  int differing = 0, worst = 0, worst_x = -1, worst_y = -1, lit = 0;

  for (int y = 0; y < height; y++) {
    row_to_rgb(frame, width, height, y, row_actual);

    if (fread(row_expected, 1, (size_t)width * 3, f) != (size_t)width * 3) {
      fprintf(stderr, "  %s is truncated at row %d\n", path, y);
      fclose(f);
      return GOLDEN_IO_ERROR;
    }

    for (int x = 0; x < width; x++) {
      const unsigned char *a = &row_actual[x * 3];
      const unsigned char *e = &row_expected[x * 3];
      if (a[0] || a[1] || a[2]) {
        lit++;
      }
      int delta = 0;
      for (int c = 0; c < 3; c++) {
        int d = a[c] - e[c];
        if (d < 0) {
          d = -d;
        }
        if (d > delta) {
          delta = d;
        }
      }
      if (delta > CHANNEL_TOLERANCE) {
        differing++;
        if (delta > worst) {
          worst = delta;
          worst_x = x;
          worst_y = y;
        }
      }
    }
  }
  fclose(f);

  if (require_content && lit < pixels * MIN_LIT_PERMILLE / 1000) {
    fprintf(stderr,
            "  only %d of %ld pixels are lit: nothing is in view, so this "
            "would compare nothing\n",
            lit, pixels);
    return GOLDEN_BLANK;
  }

  if (differing > allowed) {
    fprintf(stderr,
            "  %d of %ld pixels differ (allowed %d); worst delta %d at "
            "(%d, %d)\n",
            differing, pixels, allowed, worst, worst_x, worst_y);
    return GOLDEN_MISMATCH;
  }

  return GOLDEN_OK;
}
