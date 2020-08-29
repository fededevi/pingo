#ifdef WIN32

#include <Windows.h>
#include <wingdi.h>
#include <gdiplus.h>

#include "windowbackend.h"

#include "../render/renderer.h"
#include "../render/frame.h"
#include "../render/pixel.h"

LPCWSTR g_szClassName = L"myWindowClass";

Vec2i size;
Vec2i pos;
Pixel * frameBuffer = 0;
COLORREF * copyBuffer = 0;

HDC windowsHDC;

#define UNUSED __attribute__((__unused__))

LRESULT CALLBACK wndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch(msg)
    {
    case WM_CLOSE:
        DestroyWindow(hwnd);
        break;
    case WM_PAINT:
    {
        if (frameBuffer != 0)
        {
            for (int y = 0; y < size.x*size.y; y++ ) {
                copyBuffer[y] = pixelToRGBA(&frameBuffer[y]);
            }

            HBITMAP map = CreateBitmap(size.x, // width
                                       size.y, // height
                                       1, // Color Planes
                                       8*4, // Size of memory for one pixel in bits (in win32 4 bytes = 4*8 bits)
                                       copyBuffer); // pointer to array

            HDC bitmapHDC = CreateCompatibleDC(windowsHDC);
            SelectObject(bitmapHDC, map); // Inserting picture into our temp HDC

            BitBlt(windowsHDC, // Destination
                   pos.x,  // x dest
                   pos.y,  // y dest
                   size.x, //width
                   size.y, // height
                   bitmapHDC, // source bitmap
                   0,   // x source
                   0,   // y source
                   SRCCOPY); // Defined DWORD to juct copy pixels.

            DeleteDC(bitmapHDC);
            DeleteObject(map);
        }
        break;
    }

    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    return 0;
}

HWND WINAPI winMain(HINSTANCE hInstance, UNUSED HINSTANCE hINSTANCE, UNUSED LPSTR lPSTR, int nCmdShow)
{
    WNDCLASSEX wc;
    HWND hwnd;

    wc.cbSize        = sizeof(WNDCLASSEX);
    wc.style         = 0;
    wc.lpfnWndProc   = wndProc;
    wc.cbClsExtra    = 0;
    wc.cbWndExtra    = 0;
    wc.hInstance     = hInstance;
    wc.hIcon         = LoadIcon(NULL, IDI_APPLICATION);
    wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
    wc.lpszMenuName  = NULL;
    wc.lpszClassName = g_szClassName;
    wc.hIconSm       = LoadIcon(NULL, IDI_APPLICATION);

    if(!RegisterClassEx(&wc))
    {
        MessageBox(NULL, L"Window Registration Failed!", L"Error!",
                   MB_ICONEXCLAMATION | MB_OK);
        return 0;
    }

    // Step 2: Creating the Window
    hwnd = CreateWindowEx(
                WS_EX_CLIENTEDGE,
                g_szClassName,
                L"Overlay Emulator - Alpitronic - FD",
                WS_OVERLAPPEDWINDOW,
                CW_USEDEFAULT,
                CW_USEDEFAULT,
                WIDTH+20, //Adjusted for borders
                HEIGHT+41, //Adjusted for borders
                NULL, NULL, hInstance, NULL);

    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    windowsHDC = GetDC(hwnd);
    return hwnd;
}

void init( UNUSED Renderer * ren, BackEnd * backEnd) {

    UNUSED WindowBackEnd * this = (WindowBackEnd *) backEnd;
    winMain(0,0,0, SW_NORMAL );
}

void beforeRender( UNUSED Renderer * ren, BackEnd * backEnd) {
    UNUSED WindowBackEnd * this = (WindowBackEnd *) backEnd;

}

void afterRender(  UNUSED Renderer * ren, BackEnd * backEnd) {
    UNUSED WindowBackEnd * this = (WindowBackEnd *) backEnd;


    MSG msg; //Handle 1 message per tick
    if ( GetMessage( &msg, NULL, 0, 0 ) ) {
        TranslateMessage( &msg );
        DispatchMessage( &msg );
    }
}

Pixel * getFrameBuffer( UNUSED Renderer * ren, BackEnd * backEnd) {
    WindowBackEnd * this = (WindowBackEnd *) backEnd;
    return this->frameBuffer;
}

void windowBackEndInit(WindowBackEnd *b, Vec2i _pos, Vec2i _size) {
    b->backend.init = &init;
    b->backend.beforeRender = &beforeRender;
    b->backend.afterRender = &afterRender;
    b->backend.getFrameBuffer = &getFrameBuffer;

    size = _size;
    copyBuffer = malloc(_size.x * _size.y * 4);

    b->frameBuffer = malloc(_size.x * _size.y * sizeof (Pixel));
    frameBuffer = b->frameBuffer;
    pos = _pos;


    b->size = _size;

}

#endif
