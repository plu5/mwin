#pragma once

#include <Windows.h>
#include <string> // std::string, std::wstring

class Edit {
 public:
    HWND hwnd = 0;
    void initialise
    (HWND parent_, HINSTANCE hinst_, int x_, int y_, int w_, int h_,
     const std::string& label_);
    std::string text();
    void resize_width(int w_);
    void paint(HDC hdc);
    void clear_and_disable();
    void populate(const std::string& text);
private:
    int x = 0, y = 0, w = 0, h = 0;
    HWND parent = 0;
    HINSTANCE hinst = 0;
    std::string label;
    std::wstring wlabel;
    int label_width = 90;
};
