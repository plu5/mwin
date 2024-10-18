#pragma once

struct Brush {
    HBRUSH handle;
    Brush(int clr) {handle = CreateSolidBrush(clr);}
    ~Brush() {DeleteObject(handle);}
};

struct CompatDc {
    HDC handle;
    HWND hwnd;
    bool initialised = false;
    CompatDc() {}
    void initialise(HDC hdc, HWND hwnd_)
    {handle = CreateCompatibleDC(hdc); hwnd = hwnd_; initialised = true;}
    CompatDc(HDC hdc, HWND hwnd_) {initialise(hdc, hwnd_);}
    ~CompatDc() {if (initialised) ReleaseDC(hwnd, handle);}
};

struct CompatBitmap {
    HBITMAP handle;
    bool initialised = false;

    void initialise(HDC hdc, int w, int h) {
        delete_if_initialised();
        handle = CreateCompatibleBitmap(hdc, w, h);
        initialised = true;
    }

    CompatBitmap() {}
    CompatBitmap(HDC hdc, int w, int h) {initialise(hdc, w, h);}
    ~CompatBitmap() {delete_if_initialised();}
    void delete_if_initialised() {if (initialised) DeleteObject(handle);}
};

struct Font {
    HFONT handle;
    bool initialised = false;

    void initialise(LOGFONT logfont) {
        delete_if_initialised();
        handle = CreateFontIndirect(&logfont);
        initialised = true;
    }

    void from_current(HDC hdc, bool italic=false) {
        delete_if_initialised();
        HFONT current = reinterpret_cast<HFONT>(GetCurrentObject(hdc, OBJ_FONT));
        LOGFONT logfont {};
        GetObject(current, sizeof(LOGFONT), &logfont);
        if (italic) logfont.lfItalic = true;
        initialise(logfont);
    }

    void delete_if_initialised() {if (initialised) DeleteObject(handle);}
    ~Font() {delete_if_initialised();}
};

inline void paint_text
(HDC hdc, const std::wstring& text, COLORREF foreground, RECT* rect,
 Font* font=nullptr) {
    HFONT prev_fn = 0;
    if (font)
        prev_fn = reinterpret_cast<HFONT>(SelectObject(hdc, font->handle));
    auto prev_bk = SetBkMode(hdc, TRANSPARENT);
    auto prev_cl = SetTextColor(hdc, foreground);
    DrawTextW(hdc, text.data(), static_cast<int>(text.size()), rect, 0);
    SetBkMode(hdc, prev_bk);
    SetTextColor(hdc, prev_cl);
    if (font) SelectObject(hdc, prev_fn);
}

inline int get_text_width
(HDC hdc, const std::wstring& text, Font* font=nullptr) {
    HFONT prev_fn = 0;
    if (font)
        prev_fn = reinterpret_cast<HFONT>(SelectObject(hdc, font->handle));
    RECT rect {};
    DrawTextW(hdc, text.data(), static_cast<int>(text.size()), &rect,
              DT_CALCRECT | DT_NOPREFIX | DT_SINGLELINE);
    if (font) SelectObject(hdc, prev_fn);
    return rect.right - rect.left;
}

// https://www.catch22.net/tuts/tips/tips-and-tricks/
inline void paint_rect(HDC hdc, RECT *rect, COLORREF foreground) {
    auto prev_cl = SetBkColor(hdc, foreground);
    ExtTextOut(hdc, 0, 0, ETO_OPAQUE, rect, L"", 0, 0);
    SetBkColor(hdc, prev_cl);
}
