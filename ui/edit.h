#pragma once

#include <Windows.h>
#include <string> // std::string, std::wstring
#include "utility/win32_painting.h" // CompatDc, CompatBitmap, paint_text, ..
#include "constants.h" // Theme

class Edit {
public:
    HWND hwnd = 0;
    int label_width = 90;
    int label_top_offset = 0;
    void initialise
    (HWND parent_, HINSTANCE hinst_, int x_, int y_, int w_, int h_,
     const std::string& label_, int label_width_=90);
    std::string text() const;
    bool empty() const;
    void resize_width(int w_);
    void reposition(int x_, int y_);
    void paint(HDC hdc);
    void clear_and_disable();
    void populate(const std::string& text);
    static LRESULT CALLBACK s_proc
    (HWND hwnd, UINT msg, WPARAM wp, LPARAM lp, UINT_PTR uid, DWORD_PTR data);
protected:
    int x = 0, y = 0, w = 0, h = 0;
    int bottom_line_h = 2;
    HWND parent = 0;
    HINSTANCE hinst = 0;
    std::string label;
    std::wstring wlabel;
    COLORREF label_fg = Theme::fg;
    COLORREF bottom_line_fg = Theme::edits_line;
    LRESULT proc(UINT msg, WPARAM wp, LPARAM lp);
    // Painting & double-buffering
    HDC hdc1 = NULL;
    CompatDc dc2;
    CompatBitmap bmp;
    void setup_paint_buffers();
};
