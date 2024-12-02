#pragma once

#include <Windows.h> // HWND, HINSTANCE, RECT, HFONT
#include <string> // std::wstring
#include "ui/tooltip.h" // Tooltip

class Status {
public:
    void initialise(HWND parent_hwnd_, HINSTANCE hinst_);
    void adjust_size(RECT rect_);
    void set(const std::wstring& s);
    void set_font(HFONT font);
    void paint(HDC hdc);
protected:
    RECT rect {}; // dynamic
    HWND parent_hwnd = NULL;
    Tooltip tooltip;
    int tooltip_width = 500;
    HINSTANCE hinst = NULL;
    HFONT font = NULL;
    std::wstring current_message = L"";
};
