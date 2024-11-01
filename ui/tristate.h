#pragma once

#include <Windows.h>
#include <string> // std::wstring

void set_trackbar_range(HWND hwnd, int min, int max, bool redraw=true);
void set_trackbar_size(HWND hwnd, int size);
HWND create_trackbar
(std::wstring caption, int x, int y, int w, int h, int id,
 HWND parent, HINSTANCE hinst, bool old_style=true, int size=0);

class Tristate {
public:
    HWND hwnd = 0;
    int label_width = 90;
    int label_top_offset = 7;
    void initialise
    (HWND parent_, HINSTANCE hinst_, int x_, int y_, int w_, int h_,
     const std::string& label_, int label_foreground_=RGB(200, 200, 200),
     int label_width_=90);
    int pos() const;
    void resize_width(int w_);
    void paint(HDC hdc);
    void clear_and_disable();
    void populate(int pos_);
protected:
    int x = 0, y = 0, w = 0, h = 0;
    HWND parent = 0;
    HINSTANCE hinst = 0;
    std::string label;
    std::wstring wlabel;
    int label_foreground = RGB(200, 200, 200);
};
