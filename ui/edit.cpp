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
         WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL,
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

LRESULT CALLBACK edit_proc
(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp, UINT_PTR uid, DWORD_PTR) {
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

    case WM_NCDESTROY:
        RemoveWindowSubclass(hwnd, edit_proc, uid);
        break;
    }
    return DefSubclassProc(hwnd, msg, wp, lp);
}

void Edit::initialise
(HWND parent_, HINSTANCE hinst_, int x_, int y_, int w_, int h_,
 const std::string& label_, int label_foreground_, int label_width_) {
    hinst = hinst_;
    parent = parent_;
    label = label_;
    wlabel = string_to_wstring(label);
    x = x_;
    y = y_;
    w = w_;
    h = h_;
    label_foreground = label_foreground_;
    label_width = label_width_;
    hwnd = create_edit(L"", x + label_width, y, w - label_width, h,
                       -1, parent, hinst);
    SetWindowSubclass(hwnd, edit_proc, static_cast<UINT_PTR>(-1), 0);
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
    rect.top += label_top_offset;
    rect.left = x;
    rect.right = x + label_width;
    SetBkMode(hdc, TRANSPARENT);
    SetTextColor(hdc, label_foreground);
    DrawTextW(hdc, wlabel.data(), static_cast<int>(wlabel.size()), &rect, 0);
}

void Edit::clear_and_disable() {
    Edit_SetText(hwnd, L"");
    Edit_Enable(hwnd, false);
}

void Edit::populate(const std::string& text) {
    Edit_Enable(hwnd, true);
    Edit_SetText(hwnd, string_to_wstring(text).data());
}
