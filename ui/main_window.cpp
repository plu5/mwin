#include "ui/main_window.h"
#include <commctrl.h>
#include <string>
#include "plog/Log.h"
#include <windowsx.h> // Edit_GetSel, Edit_SetSel
#include "utility/win32_geometry.h" // get_size
#include "constants.h" // ID::name
#include "utility/win32_painting.h" // paint_rect

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
        trigger_section.adjust_size();
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

    case UM::rule_deselected:
        if (rules_list.no_rule_selected()) rule_details.clear_and_disable();
        break;

    case UM::grabbed:
        rule_details.post_grab();
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
    rule_details.initialise(hwnd, rules_list.height, trigger_section.height);
    trigger_section.initialise(hwnd, hinst);
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
                } else if (nml->uNewState == 0) {
                    PostMessage(hwnd, UM::rule_deselected, 0, 0);
                }
            }
        }
        break;
    }
}

void MainWindow::command(WPARAM wp, LPARAM lp) {
    if (not lp) return;
    if (rules_list.selected_index() != -1) {
        auto change = rule_details.command(wp, lp);
        rules_list.modify_selected_rule_field(change);
        // TODO: refactor
        if (HIWORD(wp) == BN_CLICKED and
              reinterpret_cast<HWND>(lp) == trigger_section.trigger_btn.hwnd)
            rule_details.trigger();
    }
}

void MainWindow::paint() {
    // Trigger section border
    auto rc = get_rect(hwnd);
    rc.top = rc.bottom - trigger_section.height;
    rc.bottom = rc.top + 1;
    paint_rect(dc2.h, Theme::border, &rc);

    super::paint();
}
