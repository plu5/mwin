#pragma once

#include <Windows.h>
#include <string> // std::wstring
#include "utility/win32_painting.h" // CompatDc, CompatBitmap

class Tristate {
public:
    HWND hwnd = 0;
    int label_width = 90;
    int label_top_offset = 7;
    void initialise
    (HWND parent_, HINSTANCE hinst_, int x_, int y_, int w_, int h_,
     const std::string& label_, int label_fg_=RGB(200, 200, 200),
     int label_width_=90, int bg_=RGB(0, 0, 0));
    int pos() const;
    void resize_width(int w_);
    void paint(HDC hdc);
    void clear_and_disable();
    void populate(int pos_);
    static LRESULT CALLBACK s_proc
    (HWND hwnd, UINT msg, WPARAM wp, LPARAM lp, UINT_PTR uid, DWORD_PTR);
protected:
    int x = 0, y = 0, w = 0, h = 0;
    HWND parent = 0;
    HINSTANCE hinst = 0;
    std::string label;
    std::wstring wlabel;
    int label_fg = RGB(200, 200, 200), bg = RGB(0, 0, 0);
    bool disabled = false;
    BLENDFUNCTION bf {0};
    int tick_width = 1, tick_height = 8, tick_y_offset = 4;
    LRESULT proc(UINT msg, WPARAM wp, LPARAM lp);
    // Painting & double-buffering
    HDC hdc1 = NULL;
    CompatDc dc2;
    CompatBitmap bmp;
    HFONT font = NULL;
    void setup_paint_buffers();
};
