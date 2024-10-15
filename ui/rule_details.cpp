#include "ui/rule_details.h"
#include <commctrl.h> // WC_EDIT
#include <windowsx.h> // Edit_SetText, Edit_GetTextLength, Edit_GetText
#include "plog/Log.h"
#include "utility/win32_casts.h" // hmenu_cast
#include "utility/win32_geometry.h" // get_size
#include "utility/string_conversion.h" // string_to_wstring, wstring_to_string

void RuleDetails::initialise(HWND parent_hwnd_, HINSTANCE hinst_, int y_) {
    parent_hwnd = parent_hwnd_;
    hinst = hinst_;
    y = y_;

    rule_name_edit.initialise
        (parent_hwnd, hinst, marg, y + marg,
         get_size(parent_hwnd).w - marg*2, 20, "Rule name:");
}

void RuleDetails::adjust_size() {
    rule_name_edit.resize_width(get_size(parent_hwnd).w - marg*2);
}

void RuleDetails::enable_events() {
    events_enabled = true;
}

void RuleDetails::disable_events() {
    events_enabled = false;
}

void RuleDetails::populate(const Rule& rule) {
    disable_events();
    Edit_Enable(rule_name_edit.hwnd, true);
    Edit_SetText(rule_name_edit.hwnd, string_to_wstring(rule.name).data());
    enable_events();
}

void RuleDetails::clear_and_disable() {
    disable_events();
    Edit_SetText(rule_name_edit.hwnd, L"");
    Edit_Enable(rule_name_edit.hwnd, false);
    enable_events();
}

RuleFieldChange RuleDetails::command(WPARAM wp, LPARAM lp) {
    if (!events_enabled) return {};
    if (reinterpret_cast<HWND>(lp) == rule_name_edit.hwnd) {
        if (HIWORD(wp) == EN_CHANGE) {
            return {RuleField::name, {rule_name_edit.text()}};
        }
    }
    return {};
}

void RuleDetails::paint(HDC hdc) {
    rule_name_edit.paint(hdc);
}
