#include "linux_framebuffer_backend.h"

#include "render/depth.h"
#include "render/pixel.h"
#include "render/renderer.h"
#include "render/texture.h"

#include <sys/types.h> 
#include <sys/stat.h> 
#include <sys/mman.h> 
#include <sys/ioctl.h> 
#include <fcntl.h> 
#include <linux/fb.h> 
#include <unistd.h> 
#include <stdio.h> 

Vec4i rect;
Vec2i totalSize;

PingoDepth * zetaBuffer;
Pixel * frameBuffer;

bool initFlag = false;

void init( Renderer * ren, BackEnd * backEnd, Vec4i _rect) {
    rect = _rect;

}

void beforeRender( Renderer * ren, BackEnd * backEnd) {
    LinuxFramebufferBackEnd * this = (LinuxFramebufferBackEnd *) backEnd;
}

void afterRender( Renderer * ren,  BackEnd * backEnd) {

}

Pixel * getFrameBuffer( Renderer * ren,  BackEnd * backEnd) {
    return frameBuffer;
}

PingoDepth * getZetaBuffer( Renderer * ren,  BackEnd * backEnd) {
    return zetaBuffer;
}



void linuxFramebufferBackEndInit( LinuxFramebufferBackEnd * this, Vec2i size) {
    totalSize = size;
    this->backend.init = &init;
    this->backend.beforeRender = &beforeRender;
    this->backend.afterRender = &afterRender;
    this->backend.getFrameBuffer = &getFrameBuffer;
    this->backend.getZetaBuffer = &getZetaBuffer;
    this->backend.drawPixel = 0;

    zetaBuffer = malloc(size.x*size.y*sizeof (PingoDepth));
    int fdScreen = open( "/dev/fb0", O_RDWR );
    frameBuffer = mmap( 0, 1376*768*4*1, PROT_READ | PROT_WRITE, MAP_SHARED, fdScreen, 0 );    
}

