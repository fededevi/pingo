#include "linux_window_backend.h"

#include "render/depth.h"
#include "render/pixel.h"
#include "render/renderer.h"
#include "render/texture.h"

#include <X11/Xlib.h>
#include <X11/Xos.h>
#include <X11/Xutil.h>

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

void init_x()
{
    if (dis != 0)
        return;
    dis = XOpenDisplay((char *) 0);
    screen = DefaultScreen(dis);
    unsigned long black, white;
    black = BlackPixel(dis, DefaultScreen(dis));
    white = BlackPixel(dis, DefaultScreen(dis));
    win = XCreateSimpleWindow(dis,
                              DefaultRootWindow(dis),
                              0,
                              0,
                              totalSize.x,
                              totalSize.y,
                              5,
                              white,
                              black);
    XSelectInput(dis, win, ExposureMask | KeyPressMask);
    XSetStandardProperties(dis, win, "My Window", "HI!", None, NULL, 0, NULL);
    XSelectInput(dis, win, ExposureMask | ButtonPressMask | KeyPressMask);
    XMapWindow(dis, win);
    gc = XCreateGC(dis, win, 0, 0);
    visual = DefaultVisual(dis, 0);
};

void init(Renderer *ren, Backend *Backend, Vec4i _rect)
{
    rect = _rect;
    init_x();
}

void beforeRender(Renderer *ren, Backend *Backend)
{
    LinuxWindowBackend *this = (LinuxWindowBackend *) Backend;
}

XImage *create_ximage(Display *display, Visual *visual, int width, int height)
{
    return XCreateImage(display, visual, 24, ZPixmap, 0, (char *) &frameBuffer[0], width, height, 32, 0);
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
        memcpy(f->frameBuffer + bottomIndex, tempBuffer + topIndex, f->size.x * sizeof(Pixel));
    }

    free(tempBuffer); // Free the temporary buffer
}

void afterRender(Renderer *ren, Backend *Backend)
{
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

Pixel *getFrameBuffer(Renderer *ren, Backend *Backend)
{
    return frameBuffer;
}

PingoDepth *getZetaBuffer(Renderer *ren, Backend *Backend)
{
    return zetaBuffer;
}

void linuxWindowBackendInit(LinuxWindowBackend *this, Vec2i size)
{
    totalSize = size;
    this->backend.init = &init;
    this->backend.beforeRender = &beforeRender;
    this->backend.afterRender = &afterRender;
    this->backend.getFrameBuffer = &getFrameBuffer;
    this->backend.getZetaBuffer = &getZetaBuffer;

    zetaBuffer = malloc(size.x * size.y * sizeof(PingoDepth));
    frameBuffer = malloc(size.x * size.y * sizeof(Pixel));
}
