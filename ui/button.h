#include <Windows.h>
#include <uxtheme.h> // SetWindowTheme

inline HWND create_btn
(std::wstring caption, int x, int y, int w, int h, int id,
 HWND parent, HINSTANCE hinst, bool old_style=true) {
    auto hwnd = CreateWindow
        (WC_BUTTON, caption.data(),
         WS_CHILD | WS_VISIBLE,
         x, y, w, h, parent, hmenu_cast(id), hinst, NULL);
    if (old_style) SetWindowTheme(hwnd, L" ", L" ");
    return hwnd;
}
