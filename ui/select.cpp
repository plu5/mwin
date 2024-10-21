#include "ui/select.h"
#include "commctrl.h" // WC_COMBOBOX
#include <uxtheme.h> // SetWindowTheme
#include <windowsx.h> // ComboBox_AddString
#include "utility/string_conversion.h" // string_to_wstring
#include "utility/win32_geometry.h" // get_size

void Select::initialise
(HWND parent_, HINSTANCE hinst_, int x_, int y_, int w_, int h_) {
    hinst = hinst_;
    parent = parent_;
    x = x_;
    y = y_;
    w = w_;
    h = h_;
    hwnd = CreateWindow
        (WC_COMBOBOX, L"",
         WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST,
         x, y, w, h, parent, NULL, hinst, NULL);
    SetWindowTheme(hwnd, L"DarkMode_Explorer", NULL);
}

void Select::add_option(std::string s) {
    ComboBox_AddString(hwnd, string_to_wstring(s).data());
}

void Select::select(int i, bool enable) {
    if (enable) ComboBox_Enable(hwnd, true);
    ComboBox_SetCurSel(hwnd, i);
}

int Select::selected() const {
    return ComboBox_GetCurSel(hwnd);
}

void Select::resize_width(int w_) {
    w = w_;
    SetWindowPos(hwnd, NULL, 0, 0, w, get_size(hwnd, false).h,
                 SWP_NOMOVE);
}

void Select::clear_and_disable() {
    select(-1, false);
    ComboBox_Enable(hwnd, false);
}
