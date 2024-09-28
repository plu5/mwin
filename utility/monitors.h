#pragma once

#include <vector>
#include <Windows.h>

struct Monitors {
    std::vector<RECT> rects;
    Monitors();
    static BOOL CALLBACK enum_callback(HMONITOR, HDC, LPRECT rect, LPARAM lp);   
};
