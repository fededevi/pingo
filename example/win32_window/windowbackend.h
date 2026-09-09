#pragma once

#include "example/common/expected.h"
#include "math/vec2.h"
#include "math/vec4.h"
#include "render/backend.h"
#include "render/target.h"
#include "render/depth.h"
#include "render/pixel.h"

#include <windows.h>

typedef struct {
  Backend backend;
  RenderTarget target;
  Vec2i size;
  Vec4i rect;

  HWND window_handle;
  HDC windows_hdc;

  HBITMAP dib_bitmap;
  void *dib_bits;
  HDC mem_dc;

  Pixel *frame_buffer;
  PingoDepth *depth_buffer;
  COLORREF *copy_buffer;
} WindowBackend;

PgError window_backend_init(WindowBackend *this, Vec2i size);
