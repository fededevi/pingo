#include <stdint.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>

#define UNICODE
#define _UNICODE

#include <windows.h>
#include <wingdi.h>

#include "windowbackend.h"
#include "render/renderer.h"

static LPCWSTR g_szClassName = L"myWindowClass";

static LRESULT CALLBACK wnd_proc(HWND hwnd, UINT msg, WPARAM w_param, LPARAM l_param)
{
    switch (msg)
    {
    case WM_CLOSE:
        DestroyWindow(hwnd);
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hwnd, msg, w_param, l_param);
    }
    return 0;
}

static HWND win_main(HINSTANCE h_instance, int nCmdShow, WindowBackend *wb)
{
    WNDCLASSEXW wc = {0};
    wc.cbSize = sizeof(WNDCLASSEXW);
    wc.lpfnWndProc = wnd_proc;
    wc.hInstance = h_instance;
    wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.lpszClassName = g_szClassName;
    wc.hIconSm = LoadIcon(NULL, IDI_APPLICATION);

    if (!RegisterClassExW(&wc))
    {
        MessageBoxW(NULL, L"Window Registration Failed!", L"Error!", MB_ICONEXCLAMATION | MB_OK);
        return 0;
    }

    HWND hwnd = CreateWindowExW(
        WS_EX_CLIENTEDGE,
        g_szClassName,
        L"Pingo renderer - Window backend",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT,
        wb->size.x + 16, wb->size.y + 39,
        NULL, NULL, h_instance, NULL);

    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    wb->window_handle = hwnd;
    wb->windows_hdc = GetDC(hwnd);

    return hwnd;
}

static void init(Renderer *ren, Backend *backend, Vec4i rect)
{
    (void)ren;
    WindowBackend *wb = (WindowBackend *)backend;
    wb->rect = rect;
}

static void before_render(Renderer *ren, Backend *backend)
{
    (void)ren;
    (void)backend;
}

static void after_render(Renderer *ren, Backend *backend)
{
    (void)ren;
    WindowBackend *wb = (WindowBackend *)backend;

    StretchDIBits(
        wb->windows_hdc,
        0, 0, wb->size.x, wb->size.y,
        0, 0, wb->size.x, wb->size.y,
        wb->frame_buffer,
        (BITMAPINFO *)&(BITMAPINFO){
            .bmiHeader = {
                .biSize = sizeof(BITMAPINFOHEADER),
                .biWidth = wb->size.x,
                .biHeight = wb->size.y,
                .biPlanes = 1,
                .biBitCount = 32,
                .biCompression = BI_RGB,
            }
        },
        DIB_RGB_COLORS,
        SRCCOPY);

    MSG msg;
    while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
}

static Pixel *get_frame_buffer(Renderer *ren, Backend *backend)
{
    (void)ren;
    return ((WindowBackend *)backend)->frame_buffer;
}

static PingoDepth *get_zeta_buffer(Renderer *ren, Backend *backend)
{
    (void)ren;
    return ((WindowBackend *)backend)->zeta_buffer;
}

void window_backend_init(WindowBackend *thiss, Vec2i size)
{
    printf("[Init] Initializing window backend with size %d x %d\n", size.x, size.y);

    thiss->backend.init = init;
    thiss->backend.beforeRender = before_render;
    thiss->backend.afterRender = after_render;
    thiss->backend.getFrameBuffer = get_frame_buffer;
    thiss->backend.getZetaBuffer = get_zeta_buffer;

    thiss->size = size;

    thiss->zeta_buffer = malloc(size.x * size.y * sizeof(PingoDepth));
    thiss->copy_buffer = malloc(size.x * size.y * sizeof(COLORREF));

    BITMAPINFO bmi = {0};
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = size.x;
    bmi.bmiHeader.biHeight = -size.y;
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 32;
    bmi.bmiHeader.biCompression = BI_RGB;

    HDC screen_dc = GetDC(NULL);
    thiss->mem_dc = CreateCompatibleDC(screen_dc);
    ReleaseDC(NULL, screen_dc);

    thiss->dib_bitmap = CreateDIBSection(thiss->mem_dc, &bmi, DIB_RGB_COLORS, &thiss->dib_bits, NULL, 0);
    SelectObject(thiss->mem_dc, thiss->dib_bitmap);
    thiss->frame_buffer = (Pixel *)thiss->dib_bits;

    win_main(GetModuleHandle(NULL), SW_SHOWNORMAL, thiss);

    printf("[Init] Window backend initialized successfully\n");
}
