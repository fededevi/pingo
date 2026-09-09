#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define UNICODE
#define _UNICODE

#include <windows.h>
#include <wingdi.h>

#include "render/renderer.h"
#include "windowbackend.h"
#include "render/state.h"
#include "render/target.h"

#include "example/common/example_backend.h"

static LPCWSTR g_szClassName = L"myWindowClass";

static LRESULT CALLBACK wnd_proc(HWND hwnd, UINT msg, WPARAM w_param,
                                 LPARAM l_param) {
  switch (msg) {
  case WM_CLOSE:
    DestroyWindow(hwnd);
    break;
  case WM_DESTROY:
    PostQuitMessage(0);
    break;
  default:
    return DefWindowProc(hwnd, msg, w_param, l_param);
  }
  if (render_target_init(&thiss->target, size, thiss->frame_buffer, thiss->zeta_buffer) != OK) {
    return pg_fail(PG_INVALID_ARGUMENT, "render target");
  }

  return 0;
}

static HWND win_main(HINSTANCE h_instance, int nCmdShow, WindowBackend *wb) {
  WNDCLASSEXW wc = {0};
  wc.cbSize = sizeof(WNDCLASSEXW);
  wc.lpfnWndProc = wnd_proc;
  wc.hInstance = h_instance;
  wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
  wc.hCursor = LoadCursor(NULL, IDC_ARROW);
  wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
  wc.lpszClassName = g_szClassName;
  wc.hIconSm = LoadIcon(NULL, IDI_APPLICATION);

  if (!RegisterClassExW(&wc)) {
    MessageBoxW(NULL, L"Window Registration Failed!", L"Error!",
                MB_ICONEXCLAMATION | MB_OK);
    return 0;
  }

  HWND hwnd = CreateWindowExW(
      WS_EX_CLIENTEDGE, g_szClassName, L"Pingo renderer - Window backend",
      WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, wb->size.x + 16,
      wb->size.y + 39, NULL, NULL, h_instance, NULL);

  if (hwnd == NULL) {
    MessageBoxW(NULL, L"Window Creation Failed!", L"Error!",
                MB_ICONEXCLAMATION | MB_OK);
    return 0;
  }

  ShowWindow(hwnd, nCmdShow);
  UpdateWindow(hwnd);

  wb->window_handle = hwnd;
  wb->windows_hdc = GetDC(hwnd);

  return hwnd;
}

static void init(Renderer *ren, Backend *backend, Vec4i rect) {
  (void)ren;
  WindowBackend *wb = (WindowBackend *)backend;
  wb->rect = rect;
}

static void before_render(Renderer *ren, Backend *backend) {
  (void)ren;
  (void)backend;
}

static void after_render(Renderer *ren, Backend *backend) {
  (void)ren;
  WindowBackend *wb = (WindowBackend *)backend;

  StretchDIBits(
      wb->windows_hdc, 0, 0, wb->size.x, wb->size.y, 0, 0, wb->size.x,
      wb->size.y, wb->frame_buffer,
      (BITMAPINFO *)&(BITMAPINFO){.bmiHeader =
                                      {
                                          .biSize = sizeof(BITMAPINFOHEADER),
                                          .biWidth = wb->size.x,
                                          .biHeight = wb->size.y,
                                          .biPlanes = 1,
                                          .biBitCount = 32,
                                          .biCompression = BI_RGB,
                                      }},
      DIB_RGB_COLORS, SRCCOPY);

  MSG msg;
  while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
    TranslateMessage(&msg);
    DispatchMessage(&msg);
  }
}

static RenderTarget *get_target(Renderer *ren, Backend *backend) {
  (void)ren;
  return &((WindowBackend *)backend)->target;
}

PgError window_backend_init(WindowBackend *thiss, Vec2i size) {
  if (thiss == NULL) {
    return pg_fail(PG_INVALID_ARGUMENT, "backend must not be NULL");
  }
  if (size.x <= 0 || size.y <= 0) {
    return pg_fail(PG_INVALID_ARGUMENT, "backend size must be positive");
  }

  printf("[Init] Initializing window backend with size %d x %d\n", size.x,
         size.y);

  thiss->backend.init = init;
  thiss->backend.beforeRender = before_render;
  thiss->backend.afterRender = after_render;
  thiss->backend.getTarget = get_target;

  thiss->size = size;

  const size_t pixels = (size_t)size.x * (size_t)size.y;

  thiss->zeta_buffer = malloc(pixels * sizeof(PingoDepth));
  if (thiss->zeta_buffer == NULL) {
    return pg_fail(PG_OUT_OF_MEMORY, "allocate depth buffer");
  }

  thiss->copy_buffer = malloc(pixels * sizeof(COLORREF));
  if (thiss->copy_buffer == NULL) {
    return pg_fail(PG_OUT_OF_MEMORY, "allocate copy buffer");
  }

  BITMAPINFO bmi = {0};
  bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
  bmi.bmiHeader.biWidth = size.x;
  bmi.bmiHeader.biHeight = -size.y;
  bmi.bmiHeader.biPlanes = 1;
  bmi.bmiHeader.biBitCount = 32;
  bmi.bmiHeader.biCompression = BI_RGB;

  HDC screen_dc = GetDC(NULL);
  if (screen_dc == NULL) {
    return pg_fail(PG_BACKEND, "GetDC for the screen");
  }
  thiss->mem_dc = CreateCompatibleDC(screen_dc);
  ReleaseDC(NULL, screen_dc);
  if (thiss->mem_dc == NULL) {
    return pg_fail(PG_BACKEND, "CreateCompatibleDC");
  }

  thiss->dib_bitmap = CreateDIBSection(thiss->mem_dc, &bmi, DIB_RGB_COLORS,
                                       &thiss->dib_bits, NULL, 0);
  if (thiss->dib_bitmap == NULL || thiss->dib_bits == NULL) {
    return pg_fail(PG_BACKEND, "CreateDIBSection");
  }
  SelectObject(thiss->mem_dc, thiss->dib_bitmap);
  thiss->frame_buffer = (Pixel *)thiss->dib_bits;

  if (win_main(GetModuleHandle(NULL), SW_SHOWNORMAL, thiss) == NULL) {
    return pg_fail(PG_BACKEND, "create the window");
  }

  printf("[Init] Window backend initialized successfully\n");
  return PG_SUCCESS;
}

PgError create_backend(Vec2i size, Backend **out) {
  if (out == NULL) {
    return pg_fail(PG_INVALID_ARGUMENT, "out must not be NULL");
  }

  // calloc so a failure part way through init leaves destroy_backend looking
  // at NULL handles rather than uninitialized ones.
  WindowBackend *wb = calloc(1, sizeof(WindowBackend));
  if (wb == NULL) {
    return pg_fail(PG_OUT_OF_MEMORY, "allocate window backend");
  }

  PgError error = window_backend_init(wb, size);
  if (pg_failed(error)) {
    destroy_backend((Backend *)wb);
    return error;
  }

  *out = (Backend *)wb;
  return PG_SUCCESS;
}

void destroy_backend(Backend *backend) {
  WindowBackend *wb = (WindowBackend *)backend;
  if (wb == NULL) {
    return;
  }

  if (wb->windows_hdc != NULL && wb->window_handle != NULL) {
    ReleaseDC(wb->window_handle, wb->windows_hdc);
  }
  if (wb->window_handle != NULL) {
    DestroyWindow(wb->window_handle);
  }
  if (wb->mem_dc != NULL) {
    DeleteDC(wb->mem_dc);
  }
  if (wb->dib_bitmap != NULL) {
    DeleteObject(wb->dib_bitmap);
  }

  // Was `free(wb->depth_buffer)`, a field that does not exist - this file did
  // not compile. frame_buffer is not freed: it points into the DIB section,
  // which DeleteObject already released.
  free(wb->zeta_buffer);
  free(wb->copy_buffer);
  free(wb);
}

void backend_sleep(int microseconds) {
  Sleep(microseconds / 1000); // Sleep takes milliseconds on Windows
}
