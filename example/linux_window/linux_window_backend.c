#include "linux_window_backend.h"

#include "render/renderer.h"
#include "render/texture.h"
#include "render/pixel.h"
#include "render/depth.h"

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>

Vec4i rect;
Vec2i totalSize;

PingoDepth * zetaBuffer;
Pixel * frameBuffer;

Display *dis = 0;
int screen;
Window win;
GC gc;
XImage * img = 0;
Visual *visual;

void init_x() {
    if (dis != 0)
        return;
    dis=XOpenDisplay((char *)0);
    screen=DefaultScreen(dis);
    unsigned long black, white;
    black = BlackPixel(dis, DefaultScreen(dis));
    white = BlackPixel(dis, DefaultScreen(dis));
    win=XCreateSimpleWindow(dis,DefaultRootWindow(dis),0,0, totalSize.x, totalSize.y, 5, white, black);
    XSelectInput(dis, win, ExposureMask | KeyPressMask);
    XSetStandardProperties(dis,win,"My Window","HI!",None,NULL,0,NULL);
    XSelectInput(dis, win, ExposureMask|ButtonPressMask|KeyPressMask);
    XMapWindow(dis, win);
    gc=XCreateGC(dis, win, 0,0);
    visual = DefaultVisual(dis, 0);
};

void init( Renderer * ren, BackEnd * backEnd, Vec4i _rect) {
    rect = _rect;
    init_x();
}

void beforeRender( Renderer * ren, BackEnd * backEnd) {
    LinuxWindowBackEnd * this = (LinuxWindowBackEnd *) backEnd;

}

XImage *create_ximage(Display *display, Visual *visual, int width, int height)
{
    return XCreateImage(display, visual, 24, ZPixmap, 0, (char*)&frameBuffer[0], width, height, 32, 0);
}

void afterRender( Renderer * ren,  BackEnd * backEnd) {
    if (!img)
    img = create_ximage(dis, visual, totalSize.x, totalSize.y);

    XEvent event;
    XNextEvent(dis, &event);
    //if (event.type == Expose)
    {
        XClearArea(dis, win, 0, 0, 1, 1, true);
        XPutImage(dis, win, gc, img, 0,0,0,0,totalSize.x, totalSize.y );
        //XSync(dis, 1);
        XFlush(dis);
    }

}

Pixel * getFrameBuffer( Renderer * ren,  BackEnd * backEnd) {
    return frameBuffer;
}

PingoDepth * getZetaBuffer( Renderer * ren,  BackEnd * backEnd) {
    return zetaBuffer;
}

void linuxWindowBackEndInit( LinuxWindowBackEnd * this, Vec2i size) {
    totalSize = size;
    this->backend.init = &init;
    this->backend.beforeRender = &beforeRender;
    this->backend.afterRender = &afterRender;
    this->backend.getFrameBuffer = &getFrameBuffer;
    this->backend.getZetaBuffer = &getZetaBuffer;
    this->backend.drawPixel = 0;

    zetaBuffer = malloc(size.x*size.y*sizeof (PingoDepth));
    frameBuffer = malloc(size.x*size.y*sizeof (Pixel));
    
}
