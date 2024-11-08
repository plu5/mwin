#include "ui/tristate.h"
#include <uxtheme.h> // SetWindowTheme
#include "utility/win32_casts.h" // hmenu_cast
#include "utility/win32_geometry.h" // get_rect, get_relative_rect
#include "utility/string_conversion.h" // string_to_wstring

void set_trackbar_range(HWND hwnd, int min, int max, bool redraw=true) {
    SendMessage(hwnd, TBM_SETRANGE,
                static_cast<WPARAM>(redraw),
                static_cast<LPARAM>(MAKELONG(min, max)));
}

// Only works if trackbar has flag TBS_FIXEDLENGTH
void set_trackbar_size(HWND hwnd, int size) {
    SendMessage(hwnd, TBM_SETTHUMBLENGTH,
                static_cast<WPARAM>(size),
                static_cast<LPARAM>(0));
}

LRESULT CALLBACK Tristate::s_proc
(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp, UINT_PTR uid, DWORD_PTR) {
    if (msg == WM_NCDESTROY) {
        RemoveWindowSubclass(hwnd, s_proc, uid);
        return DefSubclassProc(hwnd, msg, wp, lp);
    }
    auto self = reinterpret_cast<Tristate*>
        (GetWindowLongPtr(hwnd, GWLP_USERDATA));
    if (self) {
        return self->proc(msg, wp, lp);
    } else {
        return DefSubclassProc(hwnd, msg, wp, lp);
    }
}

void Tristate::setup_paint_buffers() {
    hdc1 = GetDC(hwnd);
    auto size = get_size(hwnd);
    bmp.initialise(hdc1, size.w, size.h);
    dc2.initialise(hdc1, hwnd);
    dc2.select_bitmap(bmp.h);
    ReleaseDC(hwnd, hdc1);
}

void draw_ticks_at
(HDC hdc, int x, int tick_w, int tick_h, int y_offset, int bottom, int clr) {
    RECT tick_rect {};
    // Top
    tick_rect.left = x;
    tick_rect.right = tick_rect.left + tick_w;
    tick_rect.top = y_offset;
    tick_rect.bottom = tick_rect.top + tick_h;
    paint_rect(hdc, clr, &tick_rect);
    // Bottom
    tick_rect.top = bottom - tick_h;
    tick_rect.bottom = tick_rect.top + tick_h;
    paint_rect(hdc, clr, &tick_rect);
}

LRESULT Tristate::proc
(UINT msg, WPARAM wp, LPARAM lp) {
    switch (msg) {
    case WM_ERASEBKGND:
        return 1;

    case WM_PAINT: {
        PAINTSTRUCT ps;
        hdc1 = BeginPaint(hwnd, &ps);
        auto size = get_size(hwnd);

        // Draw trackbar
        DefSubclassProc(hwnd, WM_PAINT, reinterpret_cast<WPARAM>(dc2.h), 0);

        // Tick x locations calculation via Castorix
        // https://stackoverflow.com/a/56607237/18396947
        RECT thumb_rect {};
        SendMessage(hwnd, TBM_GETTHUMBRECT, 0,
                    reinterpret_cast<LPARAM>(&thumb_rect));
        auto thumb_w = thumb_rect.right - thumb_rect.left;
        auto tick_rect = size.rect;
        int first_x = thumb_w + 2, mid_x = size.w / 2,
            last_x = size.rect.right - (thumb_w + 2 + 1);
        for (int tick_x : {first_x, mid_x, last_x})
            draw_ticks_at(dc2.h, tick_x, tick_width, tick_height,
                      tick_y_offset, size.rect.bottom, label_foreground);

        BitBlt(hdc1, 0, 0, size.w, size.h, dc2.h, 0, 0, SRCCOPY);
        EndPaint(hwnd, &ps);
        return 1;
    }
    }
    return DefSubclassProc(hwnd, msg, wp, lp);
}

void Tristate::initialise
(HWND parent_, HINSTANCE hinst_, int x_, int y_, int w_, int h_,
 const std::string& label_, int label_foreground_, int label_width_) {
    parent = parent_;
    hinst = hinst_;
    x = x_;
    y = y_;
    w = w_;
    h = h_;
    label = label_;
    wlabel = string_to_wstring(label);
    label_foreground = label_foreground_;
    label_width = label_width_;

    hwnd = CreateWindow
        (TRACKBAR_CLASS, L"",
         WS_CHILD | WS_VISIBLE | TBS_BOTH | TBS_FIXEDLENGTH,
         x + label_width, y, w - label_width, h,
         parent, hmenu_cast(-1), hinst, nullptr);
    SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LPARAM>(this));
    SetWindowTheme(hwnd, L" ", L" ");
    set_trackbar_size(hwnd, 15);
    SetWindowSubclass(hwnd, s_proc, static_cast<UINT_PTR>(-1), 0);
    set_trackbar_range(hwnd, 0, 2);

    setup_paint_buffers();
}

void Tristate::paint(HDC hdc) {
    auto parent_rect = get_rect(parent);
    auto rect = get_relative_rect(hwnd, parent);
    if (rect.top + (rect.bottom - rect.top) < 0) return; // not in view
    rect.top += label_top_offset;
    rect.left = x;
    rect.right = x + label_width;
    SetBkMode(hdc, TRANSPARENT);
    SetTextColor(hdc, label_foreground);
    DrawTextW(hdc, wlabel.data(), static_cast<int>(wlabel.size()), &rect, 0);
}

int Tristate::pos() const {
    return static_cast<int>(SendMessage(hwnd, TBM_GETPOS, NULL, NULL));
}

void Tristate::resize_width(int w_) {
    w = w_;
    SetWindowPos(hwnd, NULL, 0, 0, w_ - label_width, get_size(hwnd, false).h,
                 SWP_NOMOVE);
    setup_paint_buffers();
}

void Tristate::clear_and_disable() {
    EnableWindow(hwnd, false);
}

void Tristate::populate(int pos_) {
    EnableWindow(hwnd, true);
    SendMessage(hwnd, TBM_SETPOS, true, pos_);
}
