#include <QCoreApplication>

#include <Windows.h>
#include <wingdi.h>
#include <gdiplus.h>

LPCWSTR g_szClassName = L"myWindowClass";

#include "render/renderer.h"
#include "render/sprite.h"

Pixel fb0[800*500];
Pixel fb1[800*500];
Vector2I frameSize = {800,500};
Renderer r;
Scene s;

HDC windowsHDC;
HDC bitmapHDC;

LRESULT CALLBACK wndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch(msg)
    {
    case WM_CLOSE:
        DestroyWindow(hwnd);
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    return 0;
}

HWND WINAPI winMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    WNDCLASSEX wc;
    HWND hwnd;

    //Step 1: Registering the Window Class
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
                L"LLDraw",
                WS_OVERLAPPEDWINDOW,
                CW_USEDEFAULT, CW_USEDEFAULT, 800, 600,
                NULL, NULL, hInstance, NULL);

    //SetWindowPos(hwnd, HWND_TOPMOST, 100,100,800,600,0);

    if(hwnd == NULL)
    {
        MessageBox(NULL, L"Window Creation Failed!", L"Error!",
                   MB_ICONEXCLAMATION | MB_OK);
        return 0;
    }

    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    return hwnd;
}

void prepareBitMap() {
    /* BITMAP*/
    COLORREF *arr = (COLORREF*) calloc(frameSize.x*frameSize.y, sizeof(COLORREF));
    HBITMAP map = CreateBitmap(frameSize.x, // width. 512 in my case
                               frameSize.y, // height
                               1, // Color Planes, unfortanutelly don't know what is it actually. Let it be 1
                               8*4, // Size of memory for one pixel in bits (in win32 4 bytes = 4*8 bits)
                               (void*) arr); // pointer to array

    bitmapHDC = CreateCompatibleDC(windowsHDC);
    SelectObject(bitmapHDC, map); // Inserting picture into our temp HDC
}

void writeBitmap(){
    BitBlt(windowsHDC, // Destination
           10,  // x and
           10,  // y - upper-left corner of place, where we'd like to copy
           frameSize.x, // width of the region
           frameSize.y, // height
           bitmapHDC, // source
           0,   // x and
           0,   // y of upper left corner  of part of the source, from where we'd like to copy
           SRCCOPY); // Defined DWORD to juct copy pixels. Watch more on msdn;
}

int main()
{
    rendererInit(&r, frameSize, fb0, fb1);

    sceneInit(&s);

    r.scene = &s;

    Frame f;
    frameInit(&f, (Vector2I){100,100}, (Pixel *)malloc(100*100*sizeof (Pixel)));


    Sprite sprite;
    spriteInit(&sprite,f, (Vector2I){50,50});
    spriteRandomize(&sprite);

    //sceneAddRenderable(&s, frameAsRenderable(&f));
    sceneAddRenderable(&s, spriteAsRenderable(&sprite));

    HWND window = winMain(nullptr,nullptr,nullptr, SW_NORMAL );
    windowsHDC = GetDC(window);
    prepareBitMap();

    while (true ) {
        sprite.position.x++;

        rendererRender(&r);
        rendererSwap(&r);
        Frame currentBuffer = rendererCurrentBuffer(&r);

        for (uint16_t y = 0; y < currentBuffer.size.y; y++ ) {
            for (uint16_t x = 0; x < currentBuffer.size.x; x++ ) {
                Pixel p = frameRead(&currentBuffer,(Vector2I){x,y});
                SetPixel(bitmapHDC,x,y, RGB(p.r,p.b,p.g) );
            }
        }

        writeBitmap();

    }


    //DeleteObject(map);
    //DeleteDC(bitmapHDC); // Deleting temp HDC

    MSG Msg;
    // Step 3: The Message Loop
    while(GetMessage(&Msg, NULL, 0, 0) > 0)
    {
        TranslateMessage(&Msg);
        DispatchMessage(&Msg);
    }
    return Msg.wParam;
}
