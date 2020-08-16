#include <QCoreApplication>

#include <Windows.h>
#include <wingdi.h>
#include <gdiplus.h>

#include "render/renderer.h"
#include "render/sprite.h"

#define HEIGHT 800
#define WIDTH 600

LPCWSTR g_szClassName = L"myWindowClass";
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
    COLORREF *arr = (COLORREF*) calloc(WIDTH*HEIGHT, sizeof(COLORREF));
    HBITMAP map = CreateBitmap(WIDTH, // width. 512 in my case
                               HEIGHT, // height
                               1, // Color Planes, unfortanutelly don't know what is it actually. Let it be 1
                               8*4, // Size of memory for one pixel in bits (in win32 4 bytes = 4*8 bits)
                               (void*) arr); // pointer to array

    bitmapHDC = CreateCompatibleDC(windowsHDC);
    SelectObject(bitmapHDC, map); // Inserting picture into our temp HDC
}

void writeBitmap(){
    BitBlt(windowsHDC, // Destination
           00,  // x and
           00,  // y - upper-left corner of place, where we'd like to copy
           WIDTH, // width of the region
           HEIGHT, // height
           bitmapHDC, // source
           0,   // x and
           0,   // y of upper left corner  of part of the source, from where we'd like to copy
           SRCCOPY); // Defined DWORD to juct copy pixels. Watch more on msdn;
}

//Renderer FrameBuffers
Pixel fb0[WIDTH][HEIGHT];
Pixel fb1[WIDTH][HEIGHT];

//Sprite FrameBuffers
Pixel spriteFrameBuffer[100][100];

int main()
{
    qDebug("test");
    //Renderer contains 2 frame buffers for switching and handles the drawing of Renderables in the current Scene
    Vector2I frameSize = {WIDTH,HEIGHT};
    Renderer r;
    rendererInit(&r, frameSize, fb0[0], fb1[0]);





    //Build a Sprite witha  Frame and  a position
    Pixel spriteFrameBuffer[100][100];

    //Init Frame for Sprite
    Frame f;
    frameInit(&f, (Vector2I){100,100}, spriteFrameBuffer[0]);

    //Create the sprite
    Sprite sprite;
    spriteInit(&sprite,f, (Vector2I){50,50});
    spriteRandomize(&sprite);

    //Contains a list of rRenderables
    Scene s;
    sceneInit(&s);
    //Assign Scene to Renderer
    rendererSetScene(&r, &s);

    //Add sprite to the scene
    //sceneAddRenderable(&s, frameAsRenderable(&f));
    sceneAddRenderable(&s, spriteAsRenderable(&sprite));

    //Windows Stuff
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

}
