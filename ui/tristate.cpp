#include "ui/tristate.h"
#include <uxtheme.h> // SetWindowTheme
#include "utility/win32_casts.h" // hmenu_cast
#include "utility/win32_geometry.h" // get_rect, get_relative_rect
#include "utility/string_conversion.h" // string_to_wstring

void set_trackbar_range(HWND hwnd, int min, int max, bool redraw) {
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

LRESULT CALLBACK trackbar_proc
(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp, UINT_PTR uid, DWORD_PTR) {
    switch (msg) {
    case WM_ERASEBKGND:
        return 1;

    case WM_NCDESTROY:
        RemoveWindowSubclass(hwnd, trackbar_proc, uid);
        break;
    }
    return DefSubclassProc(hwnd, msg, wp, lp);
}

HWND create_trackbar
(std::wstring caption, int x, int y, int w, int h, int id,
 HWND parent, HINSTANCE hinst, bool old_style, int size) {
    auto flags = WS_CHILD | WS_VISIBLE | TBS_AUTOTICKS | TBS_BOTH | TBS_NOTIFYBEFOREMOVE;
    if (size > 0) flags |= TBS_FIXEDLENGTH;
    auto hwnd = CreateWindow
        (TRACKBAR_CLASS, caption.data(), flags,
         x, y, w, h, parent, hmenu_cast(id), hinst, NULL);
    if (old_style) SetWindowTheme(hwnd, L" ", L" ");
    if (size > 0) set_trackbar_size(hwnd, size);
    SetWindowSubclass(hwnd, trackbar_proc, static_cast<UINT_PTR>(-1), 0);
    return hwnd;
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
    hwnd = create_trackbar(L"", x + label_width, y, w - label_width, h,
                           -1, parent, hinst, true, 15);
    set_trackbar_range(hwnd, 0, 2);
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
}

void Tristate::clear_and_disable() {
    EnableWindow(hwnd, false);
}

void Tristate::populate(int pos_) {
    EnableWindow(hwnd, true);
    SendMessage(hwnd, TBM_SETPOS, true, pos_);
}
