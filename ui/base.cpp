#include "base.h"

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
    case WM_NCDESTROY:
        lres = DefWindowProc(hwnd, msg, wp, lp);
        SetWindowLongPtr(hwnd, GWLP_USERDATA, 0);
        return lres;
    }
    return DefWindowProc(hwnd, msg, wp, lp);
}
