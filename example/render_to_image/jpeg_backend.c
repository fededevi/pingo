#include "jpeg_backend.h"
#include "render/target.h"

#include "example/common/example_backend.h"

#include "render/depth.h"
#include "render/pixel.h"
#include "render/state.h"

#include <stdio.h>
#include <stdlib.h>

#include <jpeglib.h>
#include <string.h>
#include <sys/time.h>
#include <unistd.h>

static PingoDepth *zetaBuffer;
static Pixel *frameBuffer;
static RenderTarget target;
Vec2i imageSize;

void jpbe_init(Renderer *ren, Backend *backend, Vec4i _rect) {
  (void)ren;
  (void)backend;
  (void)_rect;
}

void jpbe_beforeRender(Renderer *ren, Backend *backend) {
  (void)ren;
  (void)backend;
}

static RenderTarget *jpbe_get_target(Renderer *ren, Backend *backend) {
  (void)ren;
  (void)backend;
  return &target;
}

void jpbe_afterRender(Renderer *ren, Backend *backend) {
  (void)ren;
  JpegBackend *jpegBackend = (JpegBackend *)backend;

  FILE *jpegFile = fopen(jpegBackend->jpegFilename, "wb");
  if (!jpegFile) {
    PgError error = pg_fail_errno(PG_IO, "open the output JPEG for writing");
    pg_error_report(error, stderr);
    fprintf(stderr, "  file: %s\n", jpegBackend->jpegFilename);
    exit(EXIT_FAILURE);
  }

  struct jpeg_compress_struct cinfo;
  struct jpeg_error_mgr jerr;

  cinfo.err = jpeg_std_error(&jerr);
  jpeg_create_compress(&cinfo);
  jpeg_stdio_dest(&cinfo, jpegFile);

  cinfo.image_width = imageSize.x;
  cinfo.image_height = imageSize.y;
  cinfo.input_components = 3; // RGB
  cinfo.in_color_space = JCS_RGB;

  jpeg_set_defaults(&cinfo);
  jpeg_set_quality(&cinfo, 75, TRUE); // Adjust quality if needed

  jpeg_start_compress(&cinfo, TRUE);

  // libjpeg is told three components, but a Pixel is not three bytes and its
  // channels are not in R,G,B order - handing it the framebuffer directly made
  // it read 3/4 of each row, skewing every subsequent one. That is what the
  // vertical striping in the output was. Row 0 is also the bottom of the
  // framebuffer, so the rows are emitted in reverse.
  JSAMPLE *row = malloc((size_t)imageSize.x * 3);
  if (row == NULL) {
    PgError error = pg_fail(PG_OUT_OF_MEMORY, "allocate a JPEG scanline");
    pg_error_report(error, stderr);
    jpeg_destroy_compress(&cinfo);
    fclose(jpegFile);
    exit(EXIT_FAILURE);
  }

  JSAMPROW row_pointer[1] = {row};
  while (cinfo.next_scanline < cinfo.image_height) {
    const Pixel *src =
        &frameBuffer[(imageSize.y - 1 - (int)cinfo.next_scanline) *
                     imageSize.x];
    for (int x = 0; x < imageSize.x; x++) {
      row[x * 3 + 0] = src[x].r;
      row[x * 3 + 1] = src[x].g;
      row[x * 3 + 2] = src[x].b;
    }
    jpeg_write_scanlines(&cinfo, row_pointer, 1);
  }
  free(row);

  jpeg_finish_compress(&cinfo);
  jpeg_destroy_compress(&cinfo);

  // fclose can fail on a full or failing disk, and libjpeg buffers, so a
  // successful compress does not by itself mean the file was written.
  if (fclose(jpegFile) != 0) {
    PgError error = pg_fail_errno(PG_IO, "write the output JPEG");
    pg_error_report(error, stderr);
    fprintf(stderr, "  file: %s\n", jpegBackend->jpegFilename);
    exit(EXIT_FAILURE);
  }

  printf("Wrote %s\n", jpegBackend->jpegFilename);

  // Exit after rendering one frame for image output
  exit(EXIT_SUCCESS);
}

PgError jpeg_backend_init(JpegBackend *this, Vec2i size, const char *filename) {
  if (this == NULL) {
    return pg_fail(PG_INVALID_ARGUMENT, "backend must not be NULL");
  }
  if (filename == NULL) {
    return pg_fail(PG_INVALID_ARGUMENT, "output filename must not be NULL");
  }
  if (size.x <= 0 || size.y <= 0) {
    return pg_fail(PG_INVALID_ARGUMENT, "backend size must be positive");
  }

  this->backend.init = &jpbe_init;
  this->backend.beforeRender = &jpbe_beforeRender;
  this->backend.afterRender = &jpbe_afterRender;
  this->backend.getTarget = &jpbe_get_target;

  // Zeroed so a failure part way through leaves destroy_backend safe pointers
  // to free rather than whatever malloc happened to return.
  this->jpegFilename = NULL;

  imageSize = size;

  this->jpegFilename = strdup(filename);
  if (this->jpegFilename == NULL) {
    return pg_fail(PG_OUT_OF_MEMORY, "copy the output filename");
  }

  const size_t pixels = (size_t)size.x * (size_t)size.y;

  zetaBuffer = malloc(pixels * sizeof(PingoDepth));
  if (zetaBuffer == NULL) {
    return pg_fail(PG_OUT_OF_MEMORY, "allocate depth buffer");
  }

  frameBuffer = malloc(pixels * sizeof(Pixel));
  if (frameBuffer == NULL) {
    return pg_fail(PG_OUT_OF_MEMORY, "allocate frame buffer");
  }

  if (render_target_init(&target, size, frameBuffer, zetaBuffer) != OK) {
    return pg_fail(PG_INVALID_ARGUMENT, "colour and depth buffers");
  }

  return PG_SUCCESS;
}

PgError create_backend(Vec2i size, Backend **out) {
  if (out == NULL) {
    return pg_fail(PG_INVALID_ARGUMENT, "out must not be NULL");
  }

  JpegBackend *jpegBackend = calloc(1, sizeof(JpegBackend));
  if (jpegBackend == NULL) {
    return pg_fail(PG_OUT_OF_MEMORY, "allocate JPEG backend");
  }

  PgError error = jpeg_backend_init(jpegBackend, size, "output.jpg");
  if (pg_failed(error)) {
    destroy_backend((Backend *)jpegBackend);
    return error;
  }

  *out = (Backend *)jpegBackend;
  return PG_SUCCESS;
}

void destroy_backend(Backend *backend) {
  JpegBackend *jpegBackend = (JpegBackend *)backend;
  if (jpegBackend != NULL) {
    free(jpegBackend->jpegFilename);
  }
  free(zetaBuffer);
  zetaBuffer = NULL;
  free(frameBuffer);
  frameBuffer = NULL;
  free(jpegBackend);
}

void backend_sleep(int microseconds) { usleep(microseconds); }
