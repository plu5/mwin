#pragma once

struct Brush {
    HBRUSH h = NULL;
    Brush(int clr) {h = CreateSolidBrush(clr);}
    ~Brush() {DeleteObject(h);}
};

struct CompatDc {
    HDC h = NULL;
    HWND hwnd = NULL;
    HBITMAP old_bmp = NULL;
    bool initialised = false;
    bool restore_old_bmp = true;
    CompatDc() {}
    void initialise(HDC hdc, HWND hwnd_)
    {h = CreateCompatibleDC(hdc); hwnd = hwnd_; initialised = true;}
    CompatDc(HDC hdc, HWND hwnd_) {initialise(hdc, hwnd_);}

    ~CompatDc() {
        if (restore_old_bmp) SelectObject(h, old_bmp);
        if (initialised) ReleaseDC(hwnd, h);
    }

    void select_bitmap(HBITMAP bmp) {
        old_bmp = static_cast<HBITMAP>(SelectObject(h, bmp));
        restore_old_bmp = true;
    }
};

struct CompatBitmap {
    HBITMAP h = NULL;
    bool initialised = false;

    void initialise(HDC hdc, int width, int height) {
        delete_if_initialised();
        h = CreateCompatibleBitmap(hdc, width, height);
        initialised = true;
    }

    CompatBitmap() {}
    CompatBitmap(HDC hdc, int width, int height) {initialise(hdc, width, height);}
    ~CompatBitmap() {delete_if_initialised();}
    void delete_if_initialised() {if (initialised) DeleteObject(h);}
};

struct Font {
    HFONT h = NULL;
    bool initialised = false;

    void initialise(LOGFONT logfont) {
        delete_if_initialised();
        h = CreateFontIndirect(&logfont);
        initialised = true;
    }

    void from_current(HDC hdc, bool italic=false) {
        delete_if_initialised();
        HFONT current = static_cast<HFONT>(GetCurrentObject(hdc, OBJ_FONT));
        LOGFONT logfont {};
        GetObject(current, sizeof(LOGFONT), &logfont);
        if (italic) logfont.lfItalic = true;
        initialise(logfont);
    }

    void delete_if_initialised() {if (initialised) DeleteObject(h);}
    ~Font() {delete_if_initialised();}
};

inline void paint_text
(HDC hdc, const std::wstring& text, COLORREF foreground, RECT* rect,
 Font* font=nullptr) {
    HFONT prev_fn = NULL;
    if (font) prev_fn = static_cast<HFONT>(SelectObject(hdc, font->h));
    auto prev_bk = SetBkMode(hdc, TRANSPARENT);
    auto prev_cl = SetTextColor(hdc, foreground);
    DrawTextW(hdc, text.data(), static_cast<int>(text.size()), rect, 0);
    SetBkMode(hdc, prev_bk);
    SetTextColor(hdc, prev_cl);
    if (font) SelectObject(hdc, prev_fn);
}

inline int get_text_width
(HDC hdc, const std::wstring& text, Font* font=nullptr) {
    HFONT prev_fn = NULL;
    if (font) prev_fn = static_cast<HFONT>(SelectObject(hdc, font->h));
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
