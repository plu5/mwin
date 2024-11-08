#include "ui/main_window.h"
#include <commctrl.h>
#include <string>
#include "plog/Log.h"
#include <windowsx.h> // Edit_GetSel, Edit_SetSel
#include "utility/win32_geometry.h" // get_size
#include "constants.h"

LRESULT MainWindow::proc(UINT msg, WPARAM wp, LPARAM lp) {
    switch (msg) {
    case WM_CREATE:
        initialise();
        PostMessage(hwnd, UM::post_init, 0, 0);
        break;

    case WM_GETMINMAXINFO: {
        auto pmmi = reinterpret_cast<PMINMAXINFO>(lp);
        pmmi->ptMinTrackSize.x = min_w;
        pmmi->ptMinTrackSize.y = min_h;
        break;
    }

    case WM_SIZE:
        rules_list.adjust_size();
        rule_details.adjust_size();
        break;

    case UM::post_init:
        post_init();

    case WM_COMMAND:
        rules_list.command(wp, lp);
        command(wp, lp);
        break;

    case WM_NOTIFY:
        notify(lp);
        break;

    case WM_CLOSE:
        finalise();
        break;

    case WM_NCDESTROY:
        PostQuitMessage(0);
        break;
    }
    return super::proc(msg, wp, lp);
}

INT_PTR CALLBACK MainWindow::s_about_proc
(HWND hwnd, UINT msg, WPARAM wp, LPARAM) {
    switch (msg) {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wp) == IDOK || LOWORD(wp) == IDCANCEL) {
            EndDialog(hwnd, LOWORD(wp));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}

void MainWindow::update_geometry() {
    const auto& g = config.window_geometry;
    int x = 0, y = 0, w = 400, h = 400, flags = 0;
    if (g.w && g.h) g.unpack(x, y, w, h);
    else flags |= SWP_NOMOVE;
    SetWindowPos(hwnd, NULL, x, y, w, h, flags);
}

void MainWindow::save_geometry() {
    auto& g = config.window_geometry;
    auto rect = RECT();
    if (GetWindowRect(hwnd, &rect)) {
        g.x = rect.left;
        g.y = rect.top;
        g.w = rect.right - rect.left;
        g.h = rect.bottom - rect.top;
    }
}

void MainWindow::initialise() {
    config.load();
    update_geometry();
    rules_list.initialise(hwnd, hinst);
    rules_list.load(config.user_dir);
    rule_details.initialise(hwnd, rules_list.height);
}

void MainWindow::post_init() {
    ShowWindow(hwnd, true);
    UpdateWindow(hwnd);
    loaded = true;
    rules_list.select_rule(0);
}

void MainWindow::finalise() {
    if (config.save_geom_on_quit) save_geometry();
    // TODO(plu5): Only save if changed
    config.save();
    rules_list.save(config.user_dir);
}

void MainWindow::notify(LPARAM lp) {
    if (not loaded) return;
    auto* nmh = reinterpret_cast<NMHDR*>(lp);
    switch (nmh->code) {
    case LVN_ITEMCHANGED:
        if (nmh->hwndFrom == rules_list.hwnd) {
            auto* nml = reinterpret_cast<NMLISTVIEW*>(nmh);
            if ((nml->uChanged & LVIF_STATE)) {
                // selected rule change
                if ((nml->uNewState & LVIS_SELECTED) and (nml->iItem != -1)) {
                    rule_details.populate(rules_list.rule_at(nml->iItem));
                } else if (nml->uNewState == 0) { // deselected
                    // Note(plu5): We end up here also when switching between
                    // rules, which is not currently a problem as there is then
                    // another message with the new selection state and
                    // rule_details.populate gets called, but it may be
                    // undesireable for this to be triggered between switches;
                    // ideally should only be when no rule is selected
                    rule_details.clear_and_disable();
                }
            }
        }
        break;
    }
}

void MainWindow::parse_menu_selections(WORD id) {
    switch (id) {
    case IDM_ABOUT:
        DialogBox
            (hinst, MAKEINTRESOURCE(IDD_ABOUTBOX), hwnd, s_about_proc);
        break;
    case IDM_EXIT:
        finalise();
        DestroyWindow(hwnd);
        break;
    }
}

void MainWindow::command(WPARAM wp, LPARAM lp) {
    if (lp == 0 and HIWORD(wp) == 0) { // menu
        parse_menu_selections(LOWORD(wp));
    } else {
        if (rules_list.selected_index() != -1) {
            auto change = rule_details.command(wp, lp);
            rules_list.modify_selected_rule_field(change);
        }
    }
}
