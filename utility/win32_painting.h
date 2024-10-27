#pragma once

#include <Windows.h>

struct Brush {
    HBRUSH h = NULL;
    inline Brush(int clr) {h = CreateSolidBrush(clr);}
    inline ~Brush() {DeleteObject(h);}
};

struct CompatDc {
    HDC h = NULL;
    HWND hwnd = NULL;
    HBITMAP old_bmp = NULL;
    bool initialised = false;
    bool restore_old_bmp = true;
    inline CompatDc() {}
    inline void initialise(HDC hdc, HWND hwnd_)
    {h = CreateCompatibleDC(hdc); hwnd = hwnd_; initialised = true;}
    inline CompatDc(HDC hdc, HWND hwnd_) {initialise(hdc, hwnd_);}

    inline ~CompatDc() {
        if (restore_old_bmp) SelectObject(h, old_bmp);
        if (initialised) ReleaseDC(hwnd, h);
    }

    inline void select_bitmap(HBITMAP bmp) {
        old_bmp = static_cast<HBITMAP>(SelectObject(h, bmp));
        restore_old_bmp = true;
    }
};

struct CompatBitmap {
    HBITMAP h = NULL;
    bool initialised = false;

    inline void initialise(HDC hdc, int width, int height) {
        delete_if_initialised();
        h = CreateCompatibleBitmap(hdc, width, height);
        initialised = true;
    }

    inline CompatBitmap() {}
    inline CompatBitmap(HDC hdc, int width, int height)
    {initialise(hdc, width, height);}
    inline ~CompatBitmap() {delete_if_initialised();}
    inline void delete_if_initialised() {if (initialised) DeleteObject(h);}
};

struct Font {
    HFONT h = NULL;
    bool initialised = false;

    inline void initialise(LOGFONT logfont) {
        delete_if_initialised();
        h = CreateFontIndirect(&logfont);
        initialised = true;
    }

    inline void initialise
    (HDC hdc, const std::wstring& face, int pt, bool bold=false) {
        LOGFONT logfont {};
        wcscpy_s(logfont.lfFaceName, face.data());
        logfont.lfHeight = -MulDiv(pt, GetDeviceCaps(hdc, LOGPIXELSY), 72);
        if (bold) logfont.lfWeight = FW_BOLD;
        initialise(logfont);
    }

    inline void from_current(HDC hdc, bool italic=false) {
        HFONT current = static_cast<HFONT>(GetCurrentObject(hdc, OBJ_FONT));
        LOGFONT logfont {};
        GetObject(current, sizeof(LOGFONT), &logfont);
        if (italic) logfont.lfItalic = true;
        initialise(logfont);
    }

    inline void delete_if_initialised() {if (initialised) DeleteObject(h);}
    inline ~Font() {delete_if_initialised();}
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

inline RECT get_text_rect
(HDC hdc, const std::wstring& text, Font* font=nullptr) {
    HFONT prev_fn = NULL;
    if (font) prev_fn = static_cast<HFONT>(SelectObject(hdc, font->h));
    RECT rect {};
    DrawTextW(hdc, text.data(), static_cast<int>(text.size()), &rect,
              DT_CALCRECT | DT_NOPREFIX | DT_SINGLELINE);
    if (font) SelectObject(hdc, prev_fn);
    return rect;
}

inline int get_text_width
(HDC hdc, const std::wstring& text, Font* font=nullptr) {
    auto rect = get_text_rect(hdc, text, font);
    return rect.right - rect.left;
}

inline int get_text_height
(HDC hdc, const std::wstring& text, Font* font=nullptr) {
    auto rect = get_text_rect(hdc, text, font);
    return rect.bottom - rect.top;
}

inline RECT get_centred_text_rect
(HDC hdc, const std::wstring& text, RECT* container, Font* font=nullptr,
 bool horizontally=true, bool vertically=true, bool bottom=false) {
    auto rect = get_text_rect(hdc, text, font);
    auto w = rect.right - rect.left;
    auto h = rect.bottom - rect.top;
    auto cw = container->right - container->left;
    auto ch = container->bottom - container->top;
    if (horizontally) OffsetRect(&rect, cw/2 - w/2, 0);
    if (vertically) OffsetRect(&rect, 0, ch/2 - h/2);
    else if (bottom) OffsetRect(&rect, 0, ch - h);
    return rect;
}

// https://www.catch22.net/tuts/tips/tips-and-tricks/
inline void paint_rect(HDC hdc, COLORREF foreground, RECT *rect) {
    auto prev_cl = SetBkColor(hdc, foreground);
    ExtTextOut(hdc, 0, 0, ETO_OPAQUE, rect, L"", 0, 0);
    SetBkColor(hdc, prev_cl);
}
