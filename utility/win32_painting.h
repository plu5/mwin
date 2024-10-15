#pragma once

struct Brush {
    HBRUSH hbr;
    Brush(int clr) {hbr = CreateSolidBrush(clr);}
    ~Brush() {DeleteObject(hbr);}
};

struct CompatDc {
    HDC hdc;
    HWND hwnd;
    bool initialised = false;
    CompatDc() {}
    void initialise(HDC hdc_, HWND hwnd_)
    {hdc = CreateCompatibleDC(hdc_); hwnd = hwnd_; initialised = true;}
    CompatDc(HDC hdc_, HWND hwnd_) {initialise(hdc_, hwnd_);}
    ~CompatDc() {if (initialised) ReleaseDC(hwnd, hdc);}
};

struct CompatBitmap {
    HBITMAP hb;
    bool initialised = false;
    CompatBitmap() {}
    void initialise(HDC hdc, int w, int h)
    {hb = CreateCompatibleBitmap(hdc, w, h); initialised = true;}
    CompatBitmap(HDC hdc, int w, int h) {initialise(hdc, w, h);}
    ~CompatBitmap() {if (initialised) DeleteObject(hb);}
};
