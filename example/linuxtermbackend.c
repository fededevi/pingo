#include "linuxtermbackend.h"

#include "../render/renderer.h"
#include "../render/texture.h"
#include "../render/pixel.h"
#include "../render/depth.h"
#include <stdio.h>
#include <stdlib.h>


Vec2i totalSize;
Depth * zetaBuffer;
Pixel * frameBuffer;

void linuxterm_backend_init_backend( Renderer * ren, BackEnd * backEnd, Vec4i _rect) {
    //Save the rect so the windows drawing code knows whhere and how to copy the rendered buffer on the window
}

void linuxterm_backend_beforeRender( Renderer * ren, BackEnd * backEnd) {
	// clear and move cursor to start point
	printf("\033[2J");
}

void linuxterm_backend_afterRender( Renderer * ren,  BackEnd * backEnd) {
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
	// an optional save frame to file function. 
	// You can use https://rawpixels.net/ to view
	/*
	FILE* file = fopen("frame.data", "wb");
	for (int y = 0; y < totalSize.y; y++ ) {
		for (int x = 0; x < totalSize.x; x++ ) {
			Pixel* pix = &frameBuffer[x + y * totalSize.x];
			fwrite(&pix->g, 1, 1, file);
		}
	}
	fclose(file);
	*/
}

Pixel * linuxterm_backend_getFrameBuffer( Renderer * ren,  BackEnd * backEnd) {
    return frameBuffer;
}

Depth * linuxterm_backend_getZetaBuffer( Renderer * ren,  BackEnd * backEnd) {
    return zetaBuffer;
}

void linuxterm_backend_init(LinuxTermBackend *this, Vec2i size)
{
    totalSize = size;
    this->backend.init = &linuxterm_backend_init_backend;
    this->backend.beforeRender = &linuxterm_backend_beforeRender;
    this->backend.afterRender = &linuxterm_backend_afterRender;
    this->backend.getFrameBuffer = &linuxterm_backend_getFrameBuffer;
    this->backend.getZetaBuffer = &linuxterm_backend_getZetaBuffer;
    this->backend.drawPixel = 0;

	zetaBuffer = malloc(size.x*size.y*sizeof (Depth));
	frameBuffer = malloc(size.x*size.y*sizeof (Pixel));
}
