#include "ui/button.h"
#include "utility/win32_text.h"

LRESULT CALLBACK Button::s_proc
(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp, UINT_PTR uid, DWORD_PTR){
    switch(msg) {
    case WM_LBUTTONDBLCLK:
        PostMessage(hwnd, WM_LBUTTONDOWN, wp, lp);
        break;

    case WM_ERASEBKGND:
        return 1;

    case WM_NCDESTROY:
        RemoveWindowSubclass(hwnd, s_proc, uid);
        return DefSubclassProc(hwnd, msg, wp, lp);
    }
    return DefSubclassProc(hwnd, msg, wp, lp);
}

LRESULT CALLBACK Button::s_parent_proc
(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp, UINT_PTR uid, DWORD_PTR data) {
    switch(msg) {
    case WM_DRAWITEM: {
        auto self = reinterpret_cast<Button*>(data);
        if (not self) break;
        auto pdis = reinterpret_cast<DRAWITEMSTRUCT*>(lp);
        if (pdis->hwndItem != self->hwnd) break;
        self->paint(pdis->hDC, pdis->rcItem, pdis->itemState);
        return 0;
    }

    case WM_NCDESTROY:
        RemoveWindowSubclass(hwnd, s_parent_proc, uid);
        return DefSubclassProc(hwnd, msg, wp, lp);
    }
    return DefSubclassProc(hwnd, msg, wp, lp);
}

void Button::paint(HDC hdc, RECT rc, UINT item_state) {
    if (not font.initialised)
        font.from_resource(hdc, font_id, font_face, font_size, true);
    auto pressed = item_state & ODS_SELECTED;
    paint_rect(hdc, pressed ? c_border_pressed : c_border, &rc);
    InflateRect(&rc, -1, -1);
    paint_gradient_rect(hdc, {rc.left, rc.top}, {rc.right, rc.bottom},
                        c_bg1, c_bg2);
    auto btn_text = get_window_wtext(hwnd);
    // TODO(plu5): Centre properly without needing hardcoded offset
    auto trc = get_centred_text_rect(hdc, btn_text, &rc, font.h);
    OffsetRect(&trc, 0, -2);
    if (pressed) OffsetRect(&trc, 1, 1);
    paint_text(hdc, btn_text, c_fg, &trc, font.h);
}

void Button::initialise
(HWND parent_, HINSTANCE hinst_, int x_, int y_, int w_, int h_,
 const std::wstring& label_) {
    parent = parent_;
    hinst = hinst_;
    x = x_;
    y = y_;
    w = w_;
    h = h_;
    label = label_;
    hwnd = create_btn(label, x, y, w, h, -1, parent, hinst, false,
                      WS_VISIBLE | BS_OWNERDRAW);
    SetWindowSubclass(hwnd, s_proc, static_cast<UINT_PTR>(-1), 0);
    SetWindowSubclass(parent, s_parent_proc, n_instance,
                      reinterpret_cast<DWORD_PTR>(this));
    n_instance++;
}

void Button::reposition(int x_, int y_) {
    x = x_;
    y = y_;
    SetWindowPos(hwnd, NULL, x, y, 0, 0, SWP_NOSIZE);
}
