#include "ui/select.h"
#include "commctrl.h" // WC_COMBOBOX
#include <windowsx.h> // ComboBox macros
#include "utility/string_conversion.h" // string_to_wstring
#include "utility/win32_geometry.h" // get_size

LRESULT CALLBACK s_edit_proc
(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp, UINT_PTR uid, DWORD_PTR data){
    auto select = reinterpret_cast<Select*>(data);
    switch(msg) {
    case WM_PAINT:
        // NOTE(plu5): The default WM_PAINT seems to trigger the cb WM_PAINT
        // (in Select::proc). If I suppress it, the cb WM_PAINT never gets
        // called. However, unless I InvalidateRect beforehand, although the cb
        // WM_PAINT gets called, its painting doesn't actually apply, it gets
        // clipped out or something.
        InvalidateRect(select->hwnd, NULL, false);
        DefSubclassProc(hwnd, msg, wp, lp);
        // "An application returns zero if it processes this message."
        return 0;

    case CB_SETCURSEL: {
        // NOTE(plu5): CB_SETCURSEL seems to paint over my cb without calling
        // WM_PAINT. This is a workaround to trigger a repaint of the cb after;
        // countering its repaint with our own repaint.
        auto ret = DefSubclassProc(hwnd, msg, wp, lp);
        InvalidateRect(select->hwnd, NULL, false);
        // "If the message is successful, the return value is the index of the
        // item selected."
        return ret;
    }

    case WM_NCDESTROY:
        RemoveWindowSubclass(hwnd, s_edit_proc, uid);
        return DefSubclassProc(hwnd, msg, wp, lp);
    }
    return DefSubclassProc(hwnd, msg, wp, lp);
}

LRESULT CALLBACK Select::s_proc
(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp, UINT_PTR uid, DWORD_PTR) {
    if (msg == WM_NCDESTROY) {
        RemoveWindowSubclass(hwnd, s_proc, uid);
        return DefSubclassProc(hwnd, msg, wp, lp);
    }
    auto self = reinterpret_cast<Select*>
        (GetWindowLongPtr(hwnd, GWLP_USERDATA));
    if (self) {
        return self->proc(msg, wp, lp);
    } else {
        return DefSubclassProc(hwnd, msg, wp, lp);
    }
}

std::wstring get_cb_text(HWND cb, int i=0) {
    if (i < 0 or i > ComboBox_GetCount(cb)) return {};
    auto length_with_terminator = ComboBox_GetLBTextLen(cb, i) + 1;
    auto buf = new wchar_t[length_with_terminator];
    ComboBox_GetLBText(cb, i, buf);
    auto text = std::wstring(buf);
    delete[] buf;
    return text;
}

LRESULT Select::proc
(UINT msg, WPARAM wp, LPARAM lp) {
    switch (msg) {
    case WM_SIZE:
        setup_paint_buffers();
        break;

    case WM_PAINT: {
        PAINTSTRUCT ps;
        hdc1 = BeginPaint(hwnd, &ps);
        auto size = get_size(hwnd);

        // border
        paint_rect(dc2.h, Theme::edits_line, &size.rect);

        // bg
        auto rc = size.rect;
        InflateRect(&rc, -1, -1);
        paint_rect(dc2.h, Theme::edits_bg, &rc);

        // text
        selected_text = get_cb_text(hwnd, ComboBox_GetCurSel(hwnd));
        rc.left += 3;
        if (!selected_text.empty())
            paint_text
                (dc2.h, selected_text, Theme::fg, &rc, get_window_font(hwnd),
                 DT_EDITCONTROL | DT_LEFT | DT_VCENTER | DT_SINGLELINE);

        // arrow
        int aw = size.h/2, aw2 = aw/2, ah = aw2,
            ax = rc.right - static_cast<int>(aw*1.2),
            ay = rc.bottom - size.h/2 - ah/2 + 1;
        POINT arrow[4] = {ax, ay, ax+aw2, ay, ax+aw, ay, ax+aw2, ay+ah};
        Polygon(dc2.h, arrow, 4);

        BitBlt(hdc1, 0, 0, size.w, size.h, dc2.h, 0, 0, SRCCOPY);
        EndPaint(hwnd, &ps);
        return 0;
    }
    }
    return DefSubclassProc(hwnd, msg, wp, lp);
}

void Select::setup_paint_buffers() {
    hdc1 = GetDC(hwnd);
    auto size = get_size(hwnd);
    bmp.initialise(hdc1, size.w, size.h);
    dc2.initialise(hdc1, hwnd);
    dc2.select_bitmap(bmp.h);
    ReleaseDC(hwnd, hdc1);
}

void Select::initialise
(HWND parent_, HINSTANCE hinst_, int x_, int y_, int w_, int h_) {
    hinst = hinst_;
    parent = parent_;
    x = x_;
    y = y_;
    w = w_;
    h = h_;
    hwnd = CreateWindow
        (WC_COMBOBOX, L"",
         WS_CHILD | WS_VISIBLE | CBS_HASSTRINGS | CBS_DROPDOWNLIST,
         x, y, w, h, parent, NULL, hinst, NULL);
    SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LPARAM>(this));
    SetWindowSubclass(hwnd, s_proc, static_cast<UINT_PTR>(-1), 0);
    setup_paint_buffers();

    COMBOBOXINFO info {sizeof(COMBOBOXINFO)};
    GetComboBoxInfo(hwnd, &info);
    SetWindowSubclass(info.hwndItem, s_edit_proc, static_cast<UINT_PTR>(-1),
                      reinterpret_cast<DWORD_PTR>(this));
}

void Select::add_option(std::string s) {
    ComboBox_AddString(hwnd, string_to_wstring(s).data());
}

void Select::select(int i, bool enable) {
    if (enable) ComboBox_Enable(hwnd, true);
    ComboBox_SetCurSel(hwnd, i - minimum_index);
}

int Select::selected() const {
    return ComboBox_GetCurSel(hwnd) + minimum_index;
}

void Select::resize_width(int w_) {
    w = w_;
    SetWindowPos(hwnd, NULL, 0, 0, w, get_size(hwnd, false).h,
                 SWP_NOMOVE);
}

void Select::clear_and_disable() {
    select(-1, false);
    ComboBox_Enable(hwnd, false);
}
