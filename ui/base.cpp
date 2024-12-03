#include "base.h"
#include "utility/win32_geometry.h" // get_size

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
        paint_bg(dc2.get());
        paint();
        break;

    case WM_CREATE: {
        auto hdc = GetDC(hwnd);
        font.initialise(hdc, font_face, 10);
        ReleaseDC(hwnd, hdc);
        set_window_font(hwnd, font.get());
    } [[fallthrough]];
    case WM_SIZE:
        setup_paint_buffers();
        break;

    case WM_ERASEBKGND:
        return 1;

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
    dc2.initialise(hdc1);
    dc2.select_bitmap(bmp.get());
    ReleaseDC(hwnd, hdc1);
}

void Window::paint_bg(HDC hdc) {
    RECT rect = {};
    if (GetUpdateRect(hwnd, &rect, false)) {
        FillRect(hdc, &rect, bg.get());
    }
}

void Window::paint() {
    PAINTSTRUCT ps;
    hdc1 = BeginPaint(hwnd, &ps);
    auto size = get_size(hwnd);
    BitBlt(hdc1, 0, 0, size.w, size.h, dc2.get(), 0, 0, SRCCOPY);
    EndPaint(hwnd, &ps);
}
