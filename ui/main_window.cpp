#include "ui/main_window.h"
#include "commctrl.h"
#include <string>

LRESULT MainWindow::proc(UINT msg, WPARAM wp, LPARAM lp) {
    switch (msg) {
    case WM_CREATE:
        initialise();
        break;

    case WM_SIZE:
        rules_list.adjust_size();

    case WM_COMMAND: {
        rules_list.command(wp, lp);
        int wm_id = LOWORD(wp);
        // Parse the menu selections
        switch (wm_id) {
        case IDM_ABOUT:
            DialogBox
                (hinst, MAKEINTRESOURCE(IDD_ABOUTBOX), hwnd, s_about_proc);
            break;
        case IDM_EXIT:
            finalise();
            DestroyWindow(hwnd);
            break;
        default:
            return DefWindowProc(hwnd, msg, wp, lp);
        }
        break;
    }

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
    int x = 0, y = 0, w = 400, h = 400, flags = SWP_SHOWWINDOW;
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
    rules_list = RulesList(hwnd, hinst);
}

void MainWindow::finalise() {
    if (config.save_geom_on_quit) save_geometry();
    // TODO(plu5): Only save if changed
    config.save();
}
