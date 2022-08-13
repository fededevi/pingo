#include "terminalbackend.h"

#include "../render/depth.h"
#include "../render/pixel.h"
#include "../render/renderer.h"
#include "../render/texture.h"
#include <stdio.h>
#include <stdlib.h>


Vec2i totalSize;
Depth * zetaBuffer;
Pixel * frameBuffer;

void terminal_backend_init_backend( Renderer * ren, BackEnd * backEnd, Vec4i _rect) {
    //Save the rect so the windows drawing code knows whhere and how to copy the rendered buffer on the window
}

void terminal_backend_beforeRender( Renderer * ren, BackEnd * backEnd) {
	// clear and move cursor to start point
	printf("\033[2J");
}

void terminal_backend_afterRender( Renderer * ren,  BackEnd * backEnd) {
    #define charSize 13
    const char scale[charSize] = " .:-i|=+*%#O@";
    for (int y = 0; y < totalSize.y; y++ ) {
        char chars[totalSize.x];
        for (int x = 0; x < totalSize.x; x++ ) {
			int index = charSize * (pixelToUInt8(&frameBuffer[x + y * totalSize.x]) / 256.0);
			chars[x] = scale[index];
			printf("%c", chars[x]);
        }
		printf("\n");
    }
}

Pixel * terminal_backend_getFrameBuffer( Renderer * ren,  BackEnd * backEnd) {
    return frameBuffer;
}

Depth * terminal_backend_getZetaBuffer( Renderer * ren,  BackEnd * backEnd) {
    return zetaBuffer;
}

void terminal_backend_init(TerminalBackend *this, Vec2i size)
{
    totalSize = size;
    this->backend.init = &terminal_backend_init_backend;
    this->backend.beforeRender = &terminal_backend_beforeRender;
    this->backend.afterRender = &terminal_backend_afterRender;
    this->backend.getFrameBuffer = &terminal_backend_getFrameBuffer;
    this->backend.getZetaBuffer = &terminal_backend_getZetaBuffer;
    this->backend.drawPixel = 0;

	zetaBuffer = malloc(size.x*size.y*sizeof (Depth));
	frameBuffer = malloc(size.x*size.y*sizeof (Pixel));
}
