#pragma once

#include <Windows.h>
#include <uxtheme.h> // SetWindowTheme
#include <string> // std::wstring
#include "utility/win32_casts.h" // hmenu_cast

inline HWND create_btn
(std::wstring caption, int x, int y, int w, int h, int id,
 HWND parent, HINSTANCE hinst, bool old_style=true, int flags=WS_VISIBLE,
 HICON icon=NULL) {
    auto hwnd = CreateWindow
        (WC_BUTTON, caption.data(),
         WS_CHILD | flags,
         x, y, w, h, parent, hmenu_cast(id), hinst, NULL);
    if (old_style) SetWindowTheme(hwnd, L" ", L" ");
    if (icon) SendMessage(hwnd, BM_SETIMAGE, static_cast<WPARAM>(IMAGE_ICON),
                          reinterpret_cast<LPARAM>(icon));
    return hwnd;
}

inline HWND create_cb
(std::wstring caption, int x, int y, int w, int h, int id,
 HWND parent, HINSTANCE hinst, bool old_style=true) {
    return create_btn(caption, x, y, w, h, id, parent, hinst, old_style,
                      WS_VISIBLE | BS_CHECKBOX | BS_AUTOCHECKBOX);
}
