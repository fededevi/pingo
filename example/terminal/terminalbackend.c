#include "terminalbackend.h"

#include "render/depth.h"
#include "render/pixel.h"
#include "render/renderer.h"
#include "render/texture.h"
#include <stdio.h>
#include <stdlib.h>


Vec2i totalSize;
PingoDepth * zetaBuffer;
Pixel * frameBuffer;

void terminal_backend_init_backend( Renderer * ren, BackEnd * backEnd, Vec4i _rect) {
}

void terminal_backend_beforeRender( Renderer * ren, BackEnd * backEnd) {
    // clear and move cursor to start point
    printf("\033[2J");
}

void terminal_backend_afterRender( Renderer * ren,  BackEnd * backEnd) {
    const char scale[] = "      ...,,,:::;;cc!!+*C##@";
    int charSize = sizeof(scale);
    for (int y = totalSize.y -1; y > 0; y-- ) {
        char chars[totalSize.x];
        for (int x = 0; x < totalSize.x; x++ ) {
            double normalValue = pixelToUInt8(&frameBuffer[x + y * totalSize.x]) / 255.0;
            normalValue += (rand() % 1000) * 0.0001;
            normalValue = (normalValue + 0.1) * (1.0/1.2);
            int index = 0.99 + charSize * normalValue;
            index = index + ((rand() % 3) - 1);
            index = index < 0 ? 0 : index >= charSize-1 ? charSize - 2: index;
            chars[x] = scale[index];
			printf("%c", chars[x]);
        }
		printf("\n");
    }
}

Pixel * terminal_backend_getFrameBuffer( Renderer * ren,  BackEnd * backEnd) {
    return frameBuffer;
}

PingoDepth * terminal_backend_getZetaBuffer( Renderer * ren,  BackEnd * backEnd) {
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

    zetaBuffer = malloc(size.x*size.y*sizeof (PingoDepth));
	frameBuffer = malloc(size.x*size.y*sizeof (Pixel));
}
