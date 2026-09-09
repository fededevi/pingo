#include "linux_window_backend.h"
#include "render/state.h"
#include "render/target.h"

#include "example/common/example_backend.h"

#include "render/depth.h"
#include "render/pixel.h"
#include "render/renderer.h"
#include "render/texture.h"

#include <X11/Xlib.h>
#include <X11/Xos.h>
#include <X11/Xutil.h>
#include <unistd.h>

Vec2i totalSize;

static PingoDepth *depth_buffer;
static Pixel *frame_buffer;
static RenderTarget target;

Display *dis = 0;
int screen;
Window win;
GC gc;
XImage *img = 0;
Visual *visual;

PgError init_x(void) {
  if (dis != 0)
    return PG_SUCCESS;
  dis = XOpenDisplay((char *)0);
  if (dis == NULL) {
    return pg_fail_hint(PG_BACKEND, "XOpenDisplay",
                        "No X display available. Check that DISPLAY is set "
                        "and an X server is running, or try the "
                        "linux_terminal or render_to_image example.");
  }
  screen = DefaultScreen(dis);
  unsigned long black, white;
  black = BlackPixel(dis, DefaultScreen(dis));
  white = BlackPixel(dis, DefaultScreen(dis));
  win = XCreateSimpleWindow(dis, DefaultRootWindow(dis), 0, 0, totalSize.x,
                            totalSize.y, 5, white, black);
  XSelectInput(dis, win, ExposureMask | KeyPressMask);
  XSetStandardProperties(dis, win, "My Window", "HI!", None, NULL, 0, NULL);
  XSelectInput(dis, win, ExposureMask | ButtonPressMask | KeyPressMask);
  XMapWindow(dis, win);
  gc = XCreateGC(dis, win, 0, 0);
  visual = DefaultVisual(dis, 0);
  return PG_SUCCESS;
}

void init(Renderer *ren, Backend *backend) {
  (void)ren;
  (void)backend;
  // The window is already open: create_backend does it, because this slot
  // returns void and so has no way to report a failure.
}

void before_render(Renderer *ren, Backend *backend) {
  (void)ren;
  (void)backend;
}

XImage *create_ximage(Display *display, Visual *visual, int width, int height) {
  return XCreateImage(display, visual, 24, ZPixmap, 0, (char *)&frame_buffer[0],
                      width, height, 32, 0);
}

void texture_flip_vertically(Texture *f) {
  if (f->size.y <= 1) {
    return; // No need to flip if there's only one row or less
  }

  int bufferSize = f->size.x * f->size.y * sizeof(Pixel);
  Pixel *tempBuffer = (Pixel *)malloc(bufferSize);

  if (!tempBuffer) {
    return;
  }

  // Copy the entire buffer to the temporary buffer
  memcpy(tempBuffer, f->pixels, bufferSize);

  // Flip the entire buffer vertically
  for (int y = 0; y < f->size.y; ++y) {
    int topIndex = y * f->size.x;
    int bottomIndex = (f->size.y - y - 1) * f->size.x;

    // Copy from the temporary buffer back to the texture buffer
    memcpy(f->pixels + bottomIndex, tempBuffer + topIndex,
           f->size.x * sizeof(Pixel));
  }

  free(tempBuffer); // Free the temporary buffer
}

void after_render(Renderer *ren, Backend *backend) {
  (void)backend;

  if (!img) {
    img = create_ximage(dis, visual, totalSize.x, totalSize.y);
  }

  texture_flip_vertically(&ren->target.color);
  XEvent event;
  XNextEvent(dis, &event);
  XClearArea(dis, win, 0, 0, 1, 1, true);
  XPutImage(dis, win, gc, img, 0, 0, 0, 0, totalSize.x, totalSize.y);
  XFlush(dis);
}

static RenderTarget *lw_get_target(Renderer *ren, Backend *backend) {
  (void)ren;
  (void)backend;
  return &target;
}

PgError linux_window_backend_init(LinuxWindowBackend *this, Vec2i size) {
  if (this == NULL) {
    return pg_fail(PG_INVALID_ARGUMENT, "backend must not be NULL");
  }
  if (size.x <= 0 || size.y <= 0) {
    return pg_fail(PG_INVALID_ARGUMENT, "backend size must be positive");
  }

  totalSize = size;
  this->backend.init = &init;
  this->backend.before_render = &before_render;
  this->backend.after_render = &after_render;
  this->backend.get_target = &lw_get_target;

  const size_t pixels = (size_t)size.x * (size_t)size.y;

  depth_buffer = malloc(pixels * sizeof(PingoDepth));
  if (depth_buffer == NULL) {
    return pg_fail(PG_OUT_OF_MEMORY, "allocate depth buffer");
  }

  frame_buffer = malloc(pixels * sizeof(Pixel));
  if (frame_buffer == NULL) {
    return pg_fail(PG_OUT_OF_MEMORY, "allocate frame buffer");
  }

  RETURN_IF_ERROR(init_x());

  if (render_target_init(&target, size, frame_buffer, depth_buffer) != OK) {
    return pg_fail(PG_INVALID_ARGUMENT, "colour and depth buffers");
  }

  return PG_SUCCESS;
}

PgError create_backend(Vec2i size, Backend **out) {
  if (out == NULL) {
    return pg_fail(PG_INVALID_ARGUMENT, "out must not be NULL");
  }

  LinuxWindowBackend *lwb = malloc(sizeof(LinuxWindowBackend));
  if (lwb == NULL) {
    return pg_fail(PG_OUT_OF_MEMORY, "allocate window backend");
  }

  PgError error = linux_window_backend_init(lwb, size);
  if (pg_failed(error)) {
    destroy_backend((Backend *)lwb);
    return error;
  }

  *out = (Backend *)lwb;
  return PG_SUCCESS;
}

void destroy_backend(Backend *backend) {
  if (dis != NULL) {
    if (gc != NULL) {
      XFreeGC(dis, gc);
      gc = NULL;
    }
    XDestroyWindow(dis, win);
    XCloseDisplay(dis);
    dis = NULL;
  }

  free(depth_buffer);
  depth_buffer = NULL;
  free(frame_buffer);
  frame_buffer = NULL;
  free(backend);
}

void backend_sleep(int microseconds) { usleep(microseconds); }
