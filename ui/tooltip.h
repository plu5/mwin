#pragma once

#include <Windows.h> // HWND, RECT
#include <string> // std::wstring
#include <commctrl.h> // TTTOOLINFO

class Tooltip {
public:
    void initialise
    (HWND parent, const std::wstring& text_, const RECT& rect,
     int max_width=-1);
    Tooltip() {};
    void delete_if_initialised();
    ~Tooltip();
    void set_max_width(int max_width);
    void change_rect(const RECT& rect);
private:
    bool initialised = false;
    HWND hwnd = NULL;
    std::wstring text = L"";
    TTTOOLINFO info {};
    Tooltip(const Tooltip&) = delete;
    Tooltip& operator=(const Tooltip&) = delete;
};
