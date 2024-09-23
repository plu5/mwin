class MainWindow : public Window {
public:
    using super = Window;
    MainWindow
    (std::wstring title, std::wstring class_name, HINSTANCE hinst, int show)
        : Window(title, class_name, hinst, show) {};
protected:
    LRESULT proc(UINT msg, WPARAM wp, LPARAM lp) override;
    static INT_PTR CALLBACK s_about_proc
    (HWND hwnd, UINT msg, WPARAM wp, LPARAM);
};

LRESULT MainWindow::proc(UINT msg, WPARAM wp, LPARAM lp) {
    switch (msg) {
    case WM_COMMAND: {
        int wm_id = LOWORD(wp);
        // Parse the menu selections
        switch (wm_id) {
        case IDM_ABOUT:
            DialogBox
                (hinst, MAKEINTRESOURCE(IDD_ABOUTBOX), hwnd, s_about_proc);
            break;
        case IDM_EXIT:
            DestroyWindow(hwnd);
            break;
        default:
            return DefWindowProc(hwnd, msg, wp, lp);
        }
        break;
    }
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
