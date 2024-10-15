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
    commentary_edit.initialise
        (parent_hwnd, hinst, marg, y + 20 + 2*marg,
         get_size(parent_hwnd).w - marg*2, 20, "Commentary:");
}

void RuleDetails::adjust_size() {
    auto w = get_size(parent_hwnd).w - marg*2;
    for (auto& edit : edits) edit->resize_width(w);
}

void RuleDetails::enable_events() {
    events_enabled = true;
}

void RuleDetails::disable_events() {
    events_enabled = false;
}

void RuleDetails::populate(const Rule& rule) {
    disable_events();
    rule_name_edit.populate(rule.name);
    commentary_edit.populate(rule.commentary);
    enable_events();
}

void RuleDetails::clear_and_disable() {
    disable_events();
    for (auto* edit : edits) edit->clear_and_disable();
    enable_events();
}

RuleFieldChange RuleDetails::command(WPARAM wp, LPARAM lp) {
    if (!events_enabled) return {};
    auto hwnd = reinterpret_cast<HWND>(lp);
    if (HIWORD(wp) == EN_CHANGE) {
        if (hwnd == rule_name_edit.hwnd) {
            return {RuleField::name, {rule_name_edit.text()}}; 
        } else if (hwnd == commentary_edit.hwnd) {
            return {RuleField::commentary, {commentary_edit.text()}}; 
        }
    }
    return {};
}

void RuleDetails::paint(HDC hdc) {
    for (auto* edit : edits) edit->paint(hdc);
}
