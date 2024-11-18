#include "ui/edit.h"
#include <commctrl.h> // WC_EDIT
#include <windowsx.h> // Edit_SetText, Edit_GetTextLength, Edit_GetText
#include <regex> // regex_replace
#include "utility/string_conversion.h" // string_to_wstring, wstring_to_string
#include "utility/win32_geometry.h" // get_size, get_rect, get_relative_rect
#include "utility/win32_casts.h" // hmenu_cast
#include "utility/win32_text.h" // get_window_text

HWND create_edit
(std::wstring caption, int x, int y, int w, int h, int id,
 HWND parent, HINSTANCE hinst) {
    return CreateWindow
        (WC_EDIT, caption.data(),
         WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL,
         x, y, w, h, parent, hmenu_cast(id), hinst, NULL);
}

const auto del_word_regex = std::regex("(\\w*[ ]?|[^ \\w]*[ ]?|[ ]*)$");

// Note(plu5): Edit_ReplaceSel macro makes the operation non-undoable whereas
// sending the message yourself you can specify
void edit_replace_sel(HWND hwnd, const std::string& text, BOOL undoable=TRUE) {
    auto wtext = string_to_wstring(text);
    SNDMSG(hwnd, EM_REPLACESEL, undoable,
           reinterpret_cast<LPARAM>(wtext.data()));
}

void edit_del_sel(HWND hwnd, BOOL undoable=TRUE) {
    SNDMSG(hwnd, EM_REPLACESEL, undoable, reinterpret_cast<LPARAM>(L""));
}

LRESULT CALLBACK Edit::s_proc
(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp, UINT_PTR uid, DWORD_PTR data) {
    if (msg == WM_NCDESTROY) {
        RemoveWindowSubclass(hwnd, s_proc, uid);
        return DefSubclassProc(hwnd, msg, wp, lp);
    }
    auto self = reinterpret_cast<Edit*>(data);
    if (self) {
        return self->proc(msg, wp, lp);
    } else {
        return DefSubclassProc(hwnd, msg, wp, lp);
    }
}

LRESULT Edit::proc
(UINT msg, WPARAM wp, LPARAM lp) {
    switch (msg) {
    case WM_CHAR:
        switch (wp) {
        case 1: // Ctrl+A
            Edit_SetSel(hwnd, 0, -1);
            break;
        case 127: // Ctrl+Backspace
            auto sel = Edit_GetSel(hwnd);
            auto text = get_window_text(hwnd);
            auto pre_caret = text.substr(0, LOWORD(sel));
            auto post_caret = text.substr(LOWORD(sel));
            std::smatch match;
            if (regex_search(pre_caret, match, del_word_regex)) {
                auto match_start = match.position();
                auto match_end = match_start + match.length();
                Edit_SetSel(hwnd, match_start, match_end);
                edit_del_sel(hwnd);
                WORD sel_start = static_cast<WORD>
                    (pre_caret.size() - match.length());
                WORD sel_end = sel_start + (HIWORD(sel) - LOWORD(sel));
                Edit_SetSel(hwnd, sel_start, sel_end);
            }
            return 0; // suppress character
        }
        break;

    case WM_SIZE:
        setup_paint_buffers();
        break;

    case WM_PAINT: {
        PAINTSTRUCT ps;
        hdc1 = BeginPaint(hwnd, &ps);
        DefSubclassProc(hwnd, msg, reinterpret_cast<WPARAM>(dc2.h), 0);
        auto size = get_size(hwnd);
        auto rect = size.rect;
        // line under
        rect.top = rect.bottom - bottom_line_h;
        paint_rect(dc2.h, bottom_line_fg, &rect);
        BitBlt(hdc1, 0, 0, size.w, size.h, dc2.h, 0, 0, SRCCOPY);
        EndPaint(hwnd, &ps);
        return 0;
    }
    }
    return DefSubclassProc(hwnd, msg, wp, lp);
}

void Edit::initialise
(HWND parent_, HINSTANCE hinst_, int x_, int y_, int w_, int h_,
 const std::string& label_, int label_width_) {
    hinst = hinst_;
    parent = parent_;
    label = label_;
    wlabel = string_to_wstring(label);
    x = x_;
    y = y_;
    w = w_;
    h = h_;
    label_width = label_width_;
    hwnd = create_edit(L"", x + label_width, y, w - label_width, h,
                       -1, parent, hinst);
    SetWindowSubclass(hwnd, s_proc, static_cast<UINT_PTR>(-1),
                      reinterpret_cast<DWORD_PTR>(this));
    setup_paint_buffers();
}

std::string Edit::text() const {
    return get_window_text(hwnd);
}

void Edit::resize_width(int w_) {
    w = w_;
    SetWindowPos(hwnd, NULL, 0, 0, w_ - label_width, get_size(hwnd, false).h,
                 SWP_NOMOVE);
}

void Edit::reposition(int x_, int y_) {
    x = x_;
    y = y_;
    SetWindowPos(hwnd, NULL, x + label_width, y, 0, 0,
                 SWP_NOSIZE);
}

void Edit::paint(HDC hdc) {
    auto rect = get_relative_rect(hwnd, parent);
    if (rect.top + (rect.bottom - rect.top) < 0) return; // not in view

    // label
    rect.top += label_top_offset;
    rect.left = x;
    rect.right = x + label_width;
    paint_text(hdc, wlabel, label_fg, &rect, get_window_font(hwnd));
}

void Edit::clear_and_disable() {
    Edit_SetText(hwnd, L"");
    Edit_Enable(hwnd, false);
}

void Edit::populate(const std::string& text) {
    Edit_Enable(hwnd, true);
    Edit_SetText(hwnd, string_to_wstring(text).data());
}

void Edit::setup_paint_buffers() {
    hdc1 = GetDC(hwnd);
    auto size = get_size(hwnd);
    bmp.initialise(hdc1, size.w, size.h);
    dc2.initialise(hdc1, hwnd);
    dc2.select_bitmap(bmp.h);
    ReleaseDC(hwnd, hdc1);
}
