#pragma once

#include <Windows.h>
#include <uxtheme.h> // SetWindowTheme
#include <string> // std::wstring
#include "utility/win32_casts.h" // hmenu_cast
#include "utility/win32_painting.h" // Font
#include "res/resource.h" // IDF_TRIGGERFONT

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

class Button {
public:
    HWND hwnd = NULL;
    void initialise
    (HWND parent_, HINSTANCE hinst_, int x_, int y_, int w_, int h_,
     const std::wstring& label_);
    void reposition(int x_, int y_);
    static LRESULT CALLBACK s_proc
    (HWND hwnd, UINT msg, WPARAM wp, LPARAM lp, UINT_PTR uid, DWORD_PTR);
    static LRESULT CALLBACK s_parent_proc
    (HWND hwnd, UINT msg, WPARAM wp, LPARAM lp, UINT_PTR uid, DWORD_PTR data);
protected:
    int x = 0, y = 0, w = 0, h = 0;
    UINT_PTR n_instance = 1;
    HWND parent = 0;
    HINSTANCE hinst = 0;
    std::wstring label;
    Font font;
    std::wstring font_face = L"Be Vietnam";
    int font_size = 10;
    int font_id  = IDF_TRIGGERFONT;
    COLORREF c_border = RGB(68, 73, 163);
    COLORREF c_border_pressed = RGB(198, 183, 242);
    COLORREF c_bg1 = RGB(68, 72, 163);
    COLORREF c_bg2 = RGB(93, 178, 226);
    COLORREF c_fg = 0x00FFFFFF;
    LRESULT proc(UINT msg, WPARAM wp, LPARAM lp);
    void paint(HDC hdc, RECT rc, UINT item_state);
};
