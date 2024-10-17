#pragma once

#include <Windows.h> // RECT, HWND

struct Size {
    int w = 0;
    int h = 0;
    RECT rect;
};

RECT get_rect(HWND hwnd, bool client=true);
RECT get_relative_rect(HWND hwnd, HWND parent);
Size get_size(HWND hwnd, bool client=true);
