#include "jpeg_backend.h"

#include "render/depth.h"
#include "render/pixel.h"
#include "render/state.h"

#include <stdio.h>
#include <stdlib.h>

#include <jpeglib.h>
#include <string.h>
#include <sys/time.h>

PingoDepth *zetaBuffer;
Pixel *frameBuffer;
Vec2i imageSize;

void jpbe_init(Renderer *ren, Backend *Backend, Vec4i _rect) {}

void jpbe_beforeRender(Renderer *ren, Backend *Backend) {}

Pixel * jpbe_getFrameBuffer(Renderer *ren, Backend *Backend) { return frameBuffer; }

PingoDepth * jpbe_getZetaBuffer(Renderer *ren, Backend *Backend) {
  return zetaBuffer;
}

void jpbe_afterRender(Renderer *ren, Backend *Backend)
{
  JpegBackend *jpegBackend = (JpegBackend *)Backend;

  FILE *jpegFile = fopen(jpegBackend->jpegFilename, "wb");
  if (!jpegFile) {
      fprintf(stderr, "Error opening %s for writing.\n", jpegBackend->jpegFilename);
      return;
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

  JSAMPROW row_pointer[1];
  while (cinfo.next_scanline < cinfo.image_height) {
      row_pointer[0] = &frameBuffer[cinfo.next_scanline * imageSize.x];
      jpeg_write_scanlines(&cinfo, row_pointer, 1);
  }

  jpeg_finish_compress(&cinfo);
  fclose(jpegFile);
  jpeg_destroy_compress(&cinfo);
}

int jpeg_backend_init(JpegBackend *this, Vec2i size, const char *filename) {
  this->backend.init = &jpbe_init;
  this->backend.beforeRender = &jpbe_beforeRender;
  this->backend.afterRender = &jpbe_afterRender;
  this->backend.getFrameBuffer = &jpbe_getFrameBuffer;
  this->backend.getZetaBuffer = &jpbe_getZetaBuffer;

  if (filename == NULL) {
    return INIT_ERROR; // Allocation failed
  }

  imageSize = size;

  this->jpegFilename = strdup(filename);

  zetaBuffer = malloc(size.x * size.y * sizeof(PingoDepth));
  frameBuffer = malloc(size.x * size.y * sizeof(Pixel));

  return OK; // Success
}
