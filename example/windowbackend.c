#ifdef WIN32

#include <Windows.h>
#include <wingdi.h>
#include <gdiplus.h>

#include "windowbackend.h"

#include "../render/renderer.h"
#include "../render/texture.h"
#include "../render/pixel.h"

LPCWSTR g_szClassName = L"myWindowClass";

Vec4i rect;
Pixel frameBuffer[1366*768];
COLORREF copyBuffer[1366*768*4];

HWND windowHandle;
HDC windowsHDC;

LRESULT CALLBACK wndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch(msg)
    {
    case WM_CLOSE:
        DestroyWindow(hwnd);
        break;
    case WM_ERASEBKGND:
        break;
    case WM_PAINT:
    {
        for (int y = 0; y < rect.z*rect.w; y++ ) {
            copyBuffer[y] = pixelToRGBA(&frameBuffer[y]);
        }

        HBITMAP map = CreateBitmap(rect.z, // width
                                   rect.w, // height
                                   1, // Color Planes
                                   8*4, // Size of memory for one pixel in bits (in win32 4 bytes = 4*8 bits)
                                   copyBuffer); // pointer to array

        HDC bitmapHDC = CreateCompatibleDC(windowsHDC);
        SelectObject(bitmapHDC, map); // Inserting picture into our temp HDC

        PAINTSTRUCT ps;
        //BeginPaint(windowHandle, &ps);
        FillRect(windowsHDC,&ps.rcPaint, (HBRUSH) (COLOR_WINDOW+1));
        BitBlt(windowsHDC, // Destination
               rect.x,  // x dest
               rect.y,  // y dest
               rect.z, //width
               rect.w, // height
               bitmapHDC, // source bitmap
               0,   // x source
               0,   // y source
               SRCCOPY); // Defined DWORD to juct copy pixels.
        ps.rcPaint = (RECT){0,0, 1366,768};
        //EndPaint(windowHandle, &ps);
        DeleteDC(bitmapHDC);
        DeleteObject(map);
    }
        break;

    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    return 0;
}

HWND WINAPI winMain(HINSTANCE hInstance,  HINSTANCE hINSTANCE,  LPSTR lPSTR, int nCmdShow)
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

void init( Renderer * ren, BackEnd * backEnd, Vec4i _rect) {
    //Save the rect so the windows drawing code knows whhere and how to copy the rendered buffer on the window
    rect = _rect;
}

void beforeRender( Renderer * ren, BackEnd * backEnd) {
    WindowBackEnd * this = (WindowBackEnd *) backEnd;
}

void afterRender( Renderer * ren,  BackEnd * backEnd) {
    //Dispatch window messages, eventually one of the messages will be a redraw and the window rect will be updated
    MSG msg;
    if ( GetMessage( &msg, NULL, 0, 0 ) ) {
        TranslateMessage( &msg );
        DispatchMessage( &msg );
    }
}

Pixel * getFrameBuffer( Renderer * ren,  BackEnd * backEnd) {
    return frameBuffer;
}

void windowBackEndInit( WindowBackEnd *b) {
    b->backend.init = &init;
    b->backend.beforeRender = &beforeRender;
    b->backend.afterRender = &afterRender;
    b->backend.getFrameBuffer = &getFrameBuffer;

    windowHandle = winMain(0,0,0, SW_NORMAL );
}

#endif
