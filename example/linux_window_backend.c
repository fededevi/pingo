#include "linux_window_backend.h"

#include "../render/renderer.h"
#include "../render/texture.h"
#include "../render/pixel.h"
#include "../render/depth.h"

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>

Vec4i rect;
Vec2i totalSize;

PingoDepth * zetaBuffer;
Pixel * frameBuffer;

Display *dis;
int screen;
Window win;
GC gc;

void init_x() {
	/* get the colors black and white (see section for details) */
	unsigned long black, white;

	/* use the information from the environment variable DISPLAY 
	   to create the X connection:
	*/	
	dis=XOpenDisplay((char *)0);
   	screen=DefaultScreen(dis);
	black=BlackPixel(dis,screen),	/* get color black */
	white=WhitePixel(dis, screen);  /* get color white */

	/* once the display is initialized, create the window.
	   This window will be have be 200 pixels across and 300 down.
	   It will have the foreground white and background black
	*/
   	win=XCreateSimpleWindow(dis,DefaultRootWindow(dis),0,0,	
		200, 300, 5, white, black);

	/* here is where some properties of the window can be set.
	   The third and fourth items indicate the name which appears
	   at the top of the window and the name of the minimized window
	   respectively.
	*/
	XSetStandardProperties(dis,win,"My Window","HI!",None,NULL,0,NULL);

	/* this routine determines which types of input are allowed in
	   the input.  see the appropriate section for details...
	*/
	XSelectInput(dis, win, ExposureMask|ButtonPressMask|KeyPressMask);

	/* create the Graphics Context */
        gc=XCreateGC(dis, win, 0,0);        

	/* here is another routine to set the foreground and background
	   colors _currently_ in use in the window.
	*/
	XSetBackground(dis,gc,white);
	XSetForeground(dis,gc,black);

	/* clear the window and bring it on top of the other windows */
	XClearWindow(dis, win);
	XMapRaised(dis, win);
};

void init( Renderer * ren, BackEnd * backEnd, Vec4i _rect) {
    rect = _rect;
	init_x();
}

void beforeRender( Renderer * ren, BackEnd * backEnd) {
    LinuxWindowBackEnd * this = (LinuxWindowBackEnd *) backEnd;

}

void afterRender( Renderer * ren,  BackEnd * backEnd) {

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
