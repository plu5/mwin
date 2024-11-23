#pragma once

#include <Windows.h>
#include <string> // std::string, std::wstring
#include "utility/win32_painting.h" // CompatDc, CompatBitmap
#include "constants.h" // Theme

class Select {
public:
    HWND hwnd = NULL;
    int x = 0, y = 0, w = 0, h = 0;
    int minimum_index = 0;
    Select() : edits_bg(Theme::edits_bg) {};
    void initialise
    (HWND parent_, HINSTANCE hinst_, int x_, int y_, int w_, int h_);
    void add_option(std::string s);
    void select(int i, bool enable=true);
    int selected() const;
    void resize_width(int w_);
    void clear_and_disable();
    static LRESULT CALLBACK s_proc
    (HWND hwnd, UINT msg, WPARAM wp, LPARAM lp, UINT_PTR uid, DWORD_PTR);
protected:
    HWND parent = NULL;
    HINSTANCE hinst = NULL;
    Brush edits_bg;
    std::wstring selected_text;
    LRESULT proc(UINT msg, WPARAM wp, LPARAM lp);
    // Painting & double-buffering
    HDC hdc1 = NULL;
    CompatDc dc2;
    CompatBitmap bmp;
    void setup_paint_buffers();
};
