#pragma once

#include <Windows.h>
#include <string> // std::string

class Edit {
 public:
    HWND hwnd = 0;
    void initialise
    (HWND parent_, HINSTANCE hinst_, int x, int y, int w, int h);
    std::string text();
private:
    HWND parent = 0;
    HINSTANCE hinst = 0;
};
