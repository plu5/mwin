#include "ui/rule_details.h"
#include <string> // wstring
#include <commctrl.h> // WC_EDIT
#include <windowsx.h> // Edit_SetText, Edit_GetTextLength, Edit_GetText
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

// NOTE(plu5): Maybe use GetWindowTextLength and GetWindowText directly instead
std::string get_edit_text(HWND hwnd) {
    std::wstring text;
    text.resize(Edit_GetTextLength(hwnd));
    Edit_GetText(hwnd, text.data(), text.capacity());
    auto res = wstring_to_string(text);
    return res;
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
