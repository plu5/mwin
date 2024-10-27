#include "base.h"
#include "utility/win32_geometry.h" // get_size
#include "utility/win32_painting.h" // paint_rect

LRESULT CALLBACK Window::s_proc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
    Window* self;
    if (msg == WM_NCCREATE) {
        auto lpcs = reinterpret_cast<LPCREATESTRUCT>(lp);
        self = reinterpret_cast<Window*>(lpcs->lpCreateParams);
        self->hwnd = hwnd;
        SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LPARAM>(self));
    } else {
        self = reinterpret_cast<Window*>(GetWindowLongPtr
                                         (hwnd, GWLP_USERDATA));
    }
    if (self) {
        return self->proc(msg, wp, lp);
    } else {
        return DefWindowProc(hwnd, msg, wp, lp);
    }
}

LRESULT Window::proc(UINT msg, WPARAM wp, LPARAM lp) {
    LRESULT lres;
    switch (msg) {
    case WM_PAINT:
        paint_bg(dc2.h);
        paint();
        break;

    case WM_CREATE:
    case WM_SIZE:
        setup_paint_buffers();
        break;

    case WM_ERASEBKGND: {
        paint_bg(reinterpret_cast<HDC>(wp));
        return 1;
    }

    case WM_NCDESTROY:
        lres = DefWindowProc(hwnd, msg, wp, lp);
        SetWindowLongPtr(hwnd, GWLP_USERDATA, 0);
        return lres;
    }
    return DefWindowProc(hwnd, msg, wp, lp);
}

void Window::setup_paint_buffers() {
    hdc1 = GetDC(hwnd);
    auto size = get_size(hwnd);
    bmp.initialise(hdc1, size.w, size.h);
    dc2.initialise(hdc1, hwnd);
    dc2.select_bitmap(bmp.h);
    ReleaseDC(hwnd, hdc1);
}

void Window::paint_bg(HDC hdc) {
    RECT rect = {};
    if (GetUpdateRect(hwnd, &rect, true)) {
        paint_rect(hdc, Theme::bg, &rect);
    }
}

void Window::paint() {
    RECT rect = {};
    if (GetUpdateRect(hwnd, &rect, true)) {
        auto w = rect.right - rect.left;
        auto h = rect.bottom - rect.top;
        PAINTSTRUCT ps;
        hdc1 = BeginPaint(hwnd, &ps);
        BitBlt(hdc1, 0, 0, w, h, dc2.h, 0, 0, SRCCOPY);
        EndPaint(hwnd, &ps);
    }
}
