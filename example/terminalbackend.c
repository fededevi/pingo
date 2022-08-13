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
}

void terminal_backend_beforeRender( Renderer * ren, BackEnd * backEnd) {
    // clear and move cursor to start point
    printf("\033[2J");
}

void terminal_backend_afterRender( Renderer * ren,  BackEnd * backEnd) {
    const char scale[] = " .,:c!|+*#@";
    int charSize = sizeof(scale);
    for (int y = 0; y < totalSize.y; y++ ) {
        char chars[totalSize.x];
        for (int x = 0; x < totalSize.x; x++ ) {
            int index = (charSize-2) * (pixelToUInt8(&frameBuffer[x + y * totalSize.x]) / 256.0);
            int rnd = (rand() % 3) - 1;
            chars[x] = scale[index - rnd + 1];
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
