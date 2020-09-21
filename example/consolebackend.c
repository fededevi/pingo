#include "consolebackend.h"
#include "windows.h"

#include "../render/renderer.h"
#include "../render/texture.h"
#include "../render/pixel.h"
#include "../render/depth.h"


Vec2i totalSize;
Depth * zetaBuffer;
Pixel * frameBuffer;

void console_backend_init_backend( Renderer * ren, BackEnd * backEnd, Vec4i _rect) {
    //Save the rect so the windows drawing code knows whhere and how to copy the rendered buffer on the window
}

void console_backend_beforeRender( Renderer * ren, BackEnd * backEnd) {
}
//HANDLE hConsole_c;

void write_symbol_in_color(HANDLE h, SHORT x, SHORT y, const char* symbol, WORD color)
{
    COORD here;
    here.X = x;
    here.Y = y;

    WORD attribute = color;
    DWORD written;
    WriteConsoleOutputAttribute(h, &attribute, 1, here, &written);
    WriteConsoleOutputCharacterA(h, symbol, 1, here, &written);
}

void console_backend_afterRender( Renderer * ren,  BackEnd * backEnd) {
    //Dispatch window messages, eventually one of the messages will be a redraw and the window rect will be updated
    HANDLE stdOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if (stdOut == INVALID_HANDLE_VALUE)
    {
        printf("Invalid handle");
        return;
    }

    #define charSize 13
    const char scale[charSize] = " .:-i|=+*%#O@";
    for (int y = 0; y < totalSize.y; y++ ) {
        char chars[totalSize.x];
        for (int x = 0; x < totalSize.x; x++ ) {
            int index = charSize * (pixelToUInt8(&frameBuffer[x + y * totalSize.x]) / 256.0);
            chars[x] = scale[index];
        }
        DWORD written;
        COORD here= {0, y};
        WriteConsoleOutputCharacterA(stdOut, chars, totalSize.x, here, &written);
    }

}

Pixel * console_backend_getFrameBuffer( Renderer * ren,  BackEnd * backEnd) {
    return frameBuffer;
}

Depth * console_backend_getZetaBuffer( Renderer * ren,  BackEnd * backEnd) {
    return zetaBuffer;
}

void console_backend_init(ConsoleBackend *this, Vec2i size)
{
    totalSize = size;
    this->backend.init = &console_backend_init_backend;
    this->backend.beforeRender = &console_backend_beforeRender;
    this->backend.afterRender = &console_backend_afterRender;
    this->backend.getFrameBuffer = &console_backend_getFrameBuffer;
    this->backend.getZetaBuffer = &console_backend_getZetaBuffer;

    zetaBuffer = malloc(size.x*size.y*sizeof (Depth));
    frameBuffer = malloc(size.x*size.y*sizeof (Pixel));
}
