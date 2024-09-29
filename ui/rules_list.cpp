#include "ui/rules_list.h"
#include <commctrl.h> // win32 listview
#include <windowsx.h> // GET_X_LPARAM, GET_Y_LPARAM
#include <string> // std::wstring
#include "utility/string_conversion.h" // string_to_wstring
#include "utility/win32_casts.h" // hmenu_cast
#include "core/save.h" // load_rules, save_rules

HWND create_listview
(std::wstring caption, int x, int y, int w, int h, int id,
 HWND parent, HINSTANCE hinst,
 DWORD extra_flags=WS_BORDER | LVS_SHOWSELALWAYS | LVS_SINGLESEL |
 LVS_AUTOARRANGE | LVS_ICON) {
    INITCOMMONCONTROLSEX icex {};
    icex.dwICC = ICC_LISTVIEW_CLASSES;
    InitCommonControlsEx(&icex);
    return CreateWindow
        (WC_LISTVIEW, caption.data(),
         WS_VISIBLE | WS_CHILD | extra_flags,
         x, y, w, h, parent, hmenu_cast(id),
         hinst, NULL);
}

void RulesList::select_rule(int index) {
    ListView_SetItemState(hwnd, index, LVIS_SELECTED, LVIS_SELECTED);
    ListView_SetItemState(hwnd, index, LVIS_FOCUSED, LVIS_FOCUSED);
}

int RulesList::selected_index() const {
    return ListView_GetNextItem(hwnd, -1, LVNI_SELECTED);
}

void add_item(HWND hwnd, const std::string& text, size_t index) {
    LVITEM item {};
    item.iItem = static_cast<int>(index);
    item.mask = LVIF_TEXT | LVIF_STATE;
    item.stateMask = LVIS_FOCUSED | LVIS_SELECTED;
    item.state = LVIS_FOCUSED | LVIS_SELECTED;
    auto name = string_to_wstring(text);
    item.pszText = name.data();
    ListView_InsertItem(hwnd, &item);
}

void RulesList::add_rule(const Rule& rule, size_t i) {
    add_item(hwnd, rule.name, i);
}

void RulesList::add_rule() {
    const auto i = rules.size();
    add_rule(rules.create_rule(), i);
}

void RulesList::dup_rule() {
    const auto sel = selected_index();
    if (sel == -1) return;
    const auto i = rules.size();
    const auto& rule = rules.duplicate_rule(sel);
    add_item(hwnd, rule.name, i);
}

void RulesList::del_rule() {
    const auto sel = selected_index();
    if (sel == -1) return;
    ListView_DeleteItem(hwnd, sel);
    rules.delete_rule(sel);
    // Adjust selection
    if (sel < rules.size()) select_rule(sel);
    else select_rule(static_cast<int>(rules.size()) - 1);
}

bool on_item(HWND hwnd, LPARAM lp) {
    LVHITTESTINFO hti {0};
    hti.pt = {GET_X_LPARAM(lp), GET_Y_LPARAM(lp)};
    hti.flags = LVHT_ONITEM;
    ListView_HitTest(hwnd, &hti);
    return hti.iItem != -1;
}

LRESULT CALLBACK listview_proc
(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp, UINT_PTR uid, DWORD_PTR) {
    switch (msg) {
    case WM_KILLFOCUS: // suppress selection going grey
        return 0;

    case WM_LBUTTONDOWN:
    case WM_RBUTTONDOWN:
    case WM_LBUTTONDBLCLK:
    case WM_RBUTTONDBLCLK:
        if (!on_item(hwnd, lp)) return 0; // suppress clicking off item
        break;

    case WM_NCDESTROY:
        RemoveWindowSubclass(hwnd, listview_proc, uid);
        break;
    }
    return DefSubclassProc(hwnd, msg, wp, lp);
}

HWND create_btn
(std::wstring caption, int x, int y, int w, int h, int id,
 HWND parent, HINSTANCE hinst) {
    return CreateWindow
        (WC_BUTTON, caption.data(),
         WS_CHILD | WS_VISIBLE,
         x, y, w, h, parent, hmenu_cast(id), hinst, NULL);
}

struct Size {
    int w = 0;
    int h = 0;
};

Size get_size(HWND hwnd, bool client=true) {
    RECT rect {};
    if (client) GetClientRect(hwnd, &rect);
    else GetWindowRect(hwnd, &rect);
    return {rect.right - rect.left, rect.bottom - rect.top};
}

void RulesList::load(std::filesystem::path user_dir) {
    load_rules(rules, user_dir);
    if (rules.size() > 0) {
        repopulate();
        select_rule(0);
    } else {
        add_rule();
    }
}

void RulesList::save(std::filesystem::path user_dir) {
    save_rules(rules, user_dir);
}

void RulesList::repopulate() {
    ListView_DeleteAllItems(hwnd);
    int i = 0;
    for (auto& rule : rules.rules) {
        add_rule(rule, i);
        i++;
    }
}

RulesList::RulesList(HWND parent_hwnd, HINSTANCE hinst)
    : parent_hwnd(parent_hwnd), hinst(hinst) {
    hwnd = create_listview
        (L"Rules list", btn_size, 0,
         get_size(parent_hwnd).w - btn_size, btn_size * 3,
         1, parent_hwnd, hinst);
    SetWindowSubclass(hwnd, listview_proc, static_cast<UINT_PTR>(1), 0);
    ListView_SetExtendedListViewStyle(hwnd, LVS_EX_ONECLICKACTIVATE);
    ListView_SetIconSpacing(hwnd, 80, 20);

    add_btn = create_btn(L"+", 0, 0, btn_size, btn_size,
                         2, parent_hwnd, hinst);
    dup_btn = create_btn(L"++", 0, btn_size, btn_size, btn_size,
                         3, parent_hwnd, hinst);
    del_btn = create_btn(L"-", 0, btn_size * 2, btn_size, btn_size,
                         4, parent_hwnd, hinst);

    SetFocus(hwnd);
}

void RulesList::adjust_size() {
    SetWindowPos(hwnd, NULL, 0, 0,
                 get_size(parent_hwnd).w - btn_size,
                 get_size(hwnd, false).h, SWP_NOMOVE);
}

void RulesList::command(WPARAM wp, LPARAM lp) {
    if (HIWORD(wp) == BN_CLICKED) {
        auto handle = reinterpret_cast<HWND>(lp);
        if (handle == add_btn) add_rule();
        else if (handle == dup_btn) dup_rule();
        else if (handle == del_btn) del_rule();
    }
}
