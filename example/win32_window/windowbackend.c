#ifdef WIN32

#include <Windows.h>
#include <wingdi.h>
#include <gdiplus.h>

#include "windowbackend.h"

#include "render/renderer.h"
#include "render/texture.h"
#include "render/pixel.h"
#include "render/depth.h"

LPCWSTR g_szClassName = L"myWindowClass";

Vec4i rect;
Vec2i total_size;

Depth *zeta_buffer;
Pixel *frame_buffer;
COLORREF *copy_buffer;

HWND window_handle;
HDC windows_hdc;

LRESULT CALLBACK wnd_proc(HWND hwnd, UINT msg, WPARAM w_param, LPARAM l_param)
{
    switch (msg)
    {
    case WM_CLOSE:
        DestroyWindow(hwnd);
        break;
    case WM_ERASEBKGND:
        break;
    case WM_PAINT:
    {
        for (int y = 0; y < rect.z * rect.w; y++)
        {
            copy_buffer[y] = pixel_to_rgba(&frame_buffer[y]);
        }

        HBITMAP map = CreateBitmap(rect.z,      // width
                                   rect.w,      // height
                                   1,           // Color Planes
                                   8 * 4,       // Size of memory for one pixel in bits
                                   copy_buffer); // pointer to array

        HDC bitmap_hdc = CreateCompatibleDC(windows_hdc);
        SelectObject(bitmap_hdc, map); // Inserting picture into our temp HDC

        PAINTSTRUCT ps;
        //BeginPaint(window_handle, &ps);
        FillRect(windows_hdc, &ps.rcPaint, (HBRUSH)(COLOR_WINDOW + 1));
        BitBlt(windows_hdc,  // Destination
               rect.x,       // x dest
               rect.y,       // y dest
               rect.z,       // width
               rect.w,       // height
               bitmap_hdc,   // source bitmap
               0,            // x source
               0,            // y source
               SRCCOPY);     // copy pixels
        ps.rcPaint = (RECT){0, 0, total_size.x, total_size.y};
        //EndPaint(window_handle, &ps);
        DeleteDC(bitmap_hdc);
        DeleteObject(map);
    }
    break;

    case WM_DESTROY:
        PostQuitMessage(0);
        break;

    default:
        return DefWindowProc(hwnd, msg, w_param, l_param);
    }
    return 0;
}

HWND WINAPI win_main(HINSTANCE h_instance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    (void)hPrevInstance;
    (void)lpCmdLine;

    WNDCLASSEX wc;
    HWND hwnd;

    wc.cbSize = sizeof(WNDCLASSEX);
    wc.style = 0;
    wc.lpfnWndProc = wnd_proc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = h_instance;
    wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.lpszMenuName = NULL;
    wc.lpszClassName = g_szClassName;
    wc.hIconSm = LoadIcon(NULL, IDI_APPLICATION);

    if (!RegisterClassEx(&wc))
    {
        MessageBox(NULL, L"Window Registration Failed!", L"Error!",
                   MB_ICONEXCLAMATION | MB_OK);
        return 0;
    }

    hwnd = CreateWindowEx(
        WS_EX_CLIENTEDGE,
        g_szClassName,
        L"Pingo renderer - Window backend",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        total_size.x + 20, // Adjusted for borders
        total_size.y + 41, // Adjusted for borders
        NULL, NULL, h_instance, NULL);

    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    windows_hdc = GetDC(hwnd);
    window_handle = hwnd;
    return hwnd;
}

void init(Renderer *ren, Backend *backend, Vec4i _rect)
{
    (void)ren;
    (void)backend;
    // Save the rect so the windows drawing code knows where and how to copy the rendered buffer on the window
    rect = _rect;
}

void before_render(Renderer *ren, Backend *backend)
{
    (void)ren;
    (void)backend;
}

void after_render(Renderer *ren, Backend *backend)
{
    (void)ren;
    (void)backend;

    // Dispatch window messages, eventually one of the messages will be a redraw and the window rect will be updated
    MSG msg;
    if (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
}

Pixel *get_frame_buffer(Renderer *ren, Backend *backend)
{
    (void)ren;
    (void)backend;
    return frame_buffer;
}

Depth *get_zeta_buffer(Renderer *ren, Backend *backend)
{
    (void)ren;
    (void)backend;
    return zeta_buffer;
}

void window_backend_init(WindowBackend *this, Vec2i size)
{
    total_size = size;
    this->backend.init = &init;
    this->backend.beforeRender = &before_render;
    this->backend.afterRender = &after_render;
    this->backend.getFrameBuffer = &get_frame_buffer;
    this->backend.getZetaBuffer = &get_zeta_buffer;
    this->backend.drawPixel = 0;

    zeta_buffer = malloc(size.x * size.y * sizeof(Depth));
    frame_buffer = malloc(size.x * size.y * sizeof(Pixel));
    copy_buffer = malloc(size.x * size.y * sizeof(COLORREF));

    window_handle = win_main(0, 0, 0, SW_NORMAL);
}

#endif
