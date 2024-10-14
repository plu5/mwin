#include "ui/rule_details.h"
#include <string> // wstring
#include <commctrl.h> // WC_EDIT
#include <windowsx.h> // Edit_SetText, Edit_GetTextLength, Edit_GetText
#include <regex> // regex_replace
#include "plog/Log.h"
#include "utility/win32_casts.h" // hmenu_cast
#include "utility/win32_geometry.h" // get_size
#include "utility/string_conversion.h" // string_to_wstring, wstring_to_string

HWND create_edit
(std::wstring caption, int x, int y, int w, int h, int id,
 HWND parent, HINSTANCE hinst) {
    return CreateWindow
        (WC_EDIT, caption.data(),
         WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL,
         x, y, w, h, parent, hmenu_cast(id), hinst, NULL);
}

HWND create_label
(std::wstring caption, int x, int y, int w, int h, int id,
 HWND parent, HINSTANCE hinst) {
    return CreateWindow
        (WC_STATIC, caption.data(),
         WS_CHILD | WS_VISIBLE,
         x, y, w, h, parent, hmenu_cast(id), hinst, NULL);
}

// NOTE(plu5): Maybe use GetWindowTextLength and GetWindowText directly instead
std::string get_edit_text(HWND hwnd) {
    std::wstring text;
    text.resize(Edit_GetTextLength(hwnd));
    Edit_GetText(hwnd, text.data(), text.capacity());
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
                WORD sel_start = pre_caret.size() - match.length();
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

void RuleDetails::initialise(HWND parent_hwnd_, HINSTANCE hinst_, int y_) {
    parent_hwnd = parent_hwnd_;
    hinst = hinst_;
    y = y_;

    auto rule_name_label = create_label
        (L"Rule name:", marg, y + marg + 2, label_width, 20, 1,
         parent_hwnd, hinst);
    rule_name_edit = create_edit
        (L"", label_width + marg, y + marg,
         get_size(parent_hwnd).w - label_width - marg*2, 20, 2,
         parent_hwnd, hinst);
    SetWindowSubclass(rule_name_edit, edit_proc, static_cast<UINT_PTR>(1), 0);
}

void RuleDetails::adjust_size() {
    SetWindowPos(rule_name_edit, NULL, 0, 0,
                 get_size(parent_hwnd).w - label_width - marg*2,
                 get_size(rule_name_edit, false).h, SWP_NOMOVE);
}

void RuleDetails::enable_events() {
    events_enabled = true;
}

void RuleDetails::disable_events() {
    events_enabled = false;
}

void RuleDetails::populate(const Rule& rule) {
    disable_events();
    Edit_Enable(rule_name_edit, true);
    Edit_SetText(rule_name_edit, string_to_wstring(rule.name).data());
    enable_events();
}

void RuleDetails::clear_and_disable() {
    disable_events();
    Edit_SetText(rule_name_edit, L"");
    Edit_Enable(rule_name_edit, false);
    enable_events();
}

RuleFieldChange RuleDetails::command(WPARAM wp, LPARAM lp) {
    if (!events_enabled) return {};
    if (reinterpret_cast<HWND>(lp) == rule_name_edit) {
        if (HIWORD(wp) == EN_CHANGE) {
            return {RuleField::name, {get_edit_text(rule_name_edit)}};
        }
    }
    return {};
}

DWORD RuleDetails::get_edit_field_sel(RuleField edit) {
    if (edit == RuleField::name) {
        return Edit_GetSel(rule_name_edit);
    } else {
        LOG_ERROR << "Unsupported edit field";
        return 0;
    }
}
void RuleDetails::set_edit_field_sel(RuleField edit, DWORD sel) {
    if (edit == RuleField::name) {
        Edit_SetSel(rule_name_edit, LOWORD(sel), HIWORD(sel));
    } else {
        LOG_ERROR << "Unsupported edit field";
    }
}
