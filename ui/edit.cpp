#include "ui/edit.h"
#include <commctrl.h> // WC_EDIT
#include <windowsx.h> // Edit_SetText, Edit_GetTextLength, Edit_GetText
#include <regex> // regex_replace
#include "utility/string_conversion.h" // string_to_wstring, wstring_to_string
#include "utility/win32_geometry.h" // get_size
#include "utility/win32_casts.h" // hmenu_cast

HWND create_edit
(std::wstring caption, int x, int y, int w, int h, int id,
 HWND parent, HINSTANCE hinst) {
    return CreateWindow
        (WC_EDIT, caption.data(),
         WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL,
         x, y, w, h, parent, hmenu_cast(id), hinst, NULL);
}

// NOTE(plu5): Maybe use GetWindowTextLength and GetWindowText directly instead
std::string get_edit_text(HWND hwnd) {
    std::wstring text;
    text.resize(Edit_GetTextLength(hwnd));
    Edit_GetText(hwnd, text.data(), static_cast<int>(text.capacity()));
    auto res = wstring_to_string(text);
    return res;
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
            auto text = get_edit_text(hwnd);
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

void Edit::initialise(HWND parent_, HINSTANCE hinst_,
                      int x, int y, int w, int h) {
    hinst = hinst_;
    parent = parent_;
    hwnd = create_edit(L"", x, y, w, h, -1, parent, hinst);
    SetWindowSubclass(hwnd, edit_proc, static_cast<UINT_PTR>(-1), 0);
}

std::string Edit::text() {
    return get_edit_text(hwnd);
}
