#include "linux_window_backend.h"

#include "example/common/example_backend.h"

#include "render/depth.h"
#include "render/pixel.h"
#include "render/renderer.h"
#include "render/texture.h"

#include <X11/Xlib.h>
#include <X11/Xos.h>
#include <X11/Xutil.h>
#include <unistd.h>

Vec4i rect;
Vec2i totalSize;

PingoDepth *zetaBuffer;
Pixel *frameBuffer;

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

void init(Renderer *ren, Backend *backend, Vec4i _rect) {
  (void)ren;
  (void)backend;
  rect = _rect;
  // The window is already open: create_backend does it, because this slot
  // returns void and so has no way to report a failure.
}

void beforeRender(Renderer *ren, Backend *backend) {
  (void)ren;
  (void)backend;
}

XImage *create_ximage(Display *display, Visual *visual, int width, int height) {
  return XCreateImage(display, visual, 24, ZPixmap, 0, (char *)&frameBuffer[0],
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
  memcpy(tempBuffer, f->frameBuffer, bufferSize);

  // Flip the entire buffer vertically
  for (int y = 0; y < f->size.y; ++y) {
    int topIndex = y * f->size.x;
    int bottomIndex = (f->size.y - y - 1) * f->size.x;

    // Copy from the temporary buffer back to the texture buffer
    memcpy(f->frameBuffer + bottomIndex, tempBuffer + topIndex,
           f->size.x * sizeof(Pixel));
  }

  free(tempBuffer); // Free the temporary buffer
}

void afterRender(Renderer *ren, Backend *backend) {
  (void)backend;

  if (!img) {
    img = create_ximage(dis, visual, totalSize.x, totalSize.y);
  }

  texture_flip_vertically(&ren->framebuffer);
  XEvent event;
  XNextEvent(dis, &event);
  XClearArea(dis, win, 0, 0, 1, 1, true);
  XPutImage(dis, win, gc, img, 0, 0, 0, 0, totalSize.x, totalSize.y);
  XFlush(dis);
}

Pixel *getFrameBuffer(Renderer *ren, Backend *backend) {
  (void)ren;
  (void)backend;

  return frameBuffer;
}

PingoDepth *getZetaBuffer(Renderer *ren, Backend *backend) {
  (void)ren;
  (void)backend;

  return zetaBuffer;
}

PgError linuxWindowBackendInit(LinuxWindowBackend *this, Vec2i size) {
  if (this == NULL) {
    return pg_fail(PG_INVALID_ARGUMENT, "backend must not be NULL");
  }
  if (size.x <= 0 || size.y <= 0) {
    return pg_fail(PG_INVALID_ARGUMENT, "backend size must be positive");
  }

  totalSize = size;
  this->backend.init = &init;
  this->backend.beforeRender = &beforeRender;
  this->backend.afterRender = &afterRender;
  this->backend.getFrameBuffer = &getFrameBuffer;
  this->backend.getZetaBuffer = &getZetaBuffer;

  const size_t pixels = (size_t)size.x * (size_t)size.y;

  zetaBuffer = malloc(pixels * sizeof(PingoDepth));
  if (zetaBuffer == NULL) {
    return pg_fail(PG_OUT_OF_MEMORY, "allocate depth buffer");
  }

  frameBuffer = malloc(pixels * sizeof(Pixel));
  if (frameBuffer == NULL) {
    return pg_fail(PG_OUT_OF_MEMORY, "allocate frame buffer");
  }

  RETURN_IF_ERROR(init_x());

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

  PgError error = linuxWindowBackendInit(lwb, size);
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

  free(zetaBuffer);
  zetaBuffer = NULL;
  free(frameBuffer);
  frameBuffer = NULL;
  free(backend);
}

void backend_sleep(int microseconds) { usleep(microseconds); }
