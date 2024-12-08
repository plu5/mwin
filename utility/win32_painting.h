#pragma once

#include <Windows.h>
#include <string> // std::wstring

struct Brush {
    HBRUSH h = NULL;
    inline Brush(int clr) {h = CreateSolidBrush(clr);}
    inline ~Brush() {DeleteObject(h);}
    // noncopyable
    Brush(const Brush&) = delete;
    Brush& operator=(const Brush&) = delete;
    // movable
    Brush(Brush&&) = default;
};

struct CompatDc {
    HDC h = NULL;
    HWND hwnd = NULL;
    HBITMAP old_bmp = NULL;
    HDC old_bmp_dc = NULL;
    bool initialised = false;
    bool restore_old_bmp = false;

    inline void initialise(HDC hdc, HWND hwnd_) {
        delete_if_initialised();
        h = CreateCompatibleDC(hdc);
        hwnd = hwnd_;
        initialised = true;
    }

    inline void delete_if_initialised() {
        if (restore_old_bmp and old_bmp and old_bmp_dc) {
            SelectObject(old_bmp_dc, old_bmp);
            restore_old_bmp = false;
            old_bmp = NULL;
            old_bmp_dc = NULL;
        }
        // A DC obtained via GetDC should be released with ReleaseDC, but a DC
        // created ourselves should be deleted, otherwise it will leak.
        if (initialised) DeleteDC(h);
    }

    inline void select_bitmap(HBITMAP bmp) {
        old_bmp = static_cast<HBITMAP>(SelectObject(h, bmp));
        old_bmp_dc = h;
        restore_old_bmp = true;
    }

    inline CompatDc() {}
    inline CompatDc(HDC hdc, HWND hwnd_) {initialise(hdc, hwnd_);}
    inline ~CompatDc() {delete_if_initialised();}
    // noncopyable
    CompatDc(const CompatDc&) = delete;
    CompatDc& operator=(const CompatDc&) = delete;
    // movable
    CompatDc(CompatDc&&) = default;
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
    // noncopyable
    CompatBitmap(const CompatBitmap&) = delete;
    CompatBitmap& operator=(const CompatBitmap&) = delete;
    // movable
    CompatBitmap(CompatBitmap&&) = default;
};

// LoadIcon loads a "shared icon" which doesn't need to be deleted.
// There is also no issue with calling it several times; if already loaded, it
// retrieves a handle to the existing resource.
// So there is not really a need to wrap it but I already did so nvm...
struct Icon {
    HICON h = NULL;

    inline void initialise(HINSTANCE hinst, int id) {
        h = LoadIcon(hinst, MAKEINTRESOURCE(id));
    }

    inline Icon() {}
    inline Icon(HINSTANCE hinst, int id)
    {initialise(hinst, id);}
};

struct Font {
    HFONT h = NULL;
    HANDLE res_h = NULL;
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

    inline void from_current(HDC hdc, bool italic=false, bool bold=false) {
        HFONT current = static_cast<HFONT>(GetCurrentObject(hdc, OBJ_FONT));
        LOGFONT logfont {};
        GetObject(current, sizeof(LOGFONT), &logfont);
        if (italic) logfont.lfItalic = true;
        if (bold) logfont.lfWeight = FW_BOLD;
        initialise(logfont);
    }

    inline void from_resource
    (HDC hdc, int id, const std::wstring& face, int pt, bool bold=false,
     const std::wstring& fallback=L"MS Dlg 2") {
        delete_if_initialised(true);
        // Adrian Mole https://stackoverflow.com/a/58713364/18396947
        HINSTANCE hinst = GetModuleHandle(nullptr);
        HRSRC res = FindResource(hinst, MAKEINTRESOURCE(id), L"BINARY");
        if (res) {
            HGLOBAL font_mem = LoadResource(hinst, res);
            if (font_mem != nullptr) {
                void* font_data = LockResource(font_mem);
                DWORD n_fonts = 0, len = SizeofResource(hinst, res);
                res_h = AddFontMemResourceEx(font_data, len, nullptr, &n_fonts);
            }
        }
        initialise(hdc, res_h ? face : fallback, pt, bold);
    }

    inline void delete_if_initialised(bool res=false) {
        if (h) DeleteObject(h);
        if (res and res_h) RemoveFontMemResourceEx(res_h);
        initialised = false;
    }

    inline Font() {};
    inline ~Font() {delete_if_initialised(true);}
    // noncopyable
    Font(const Font&) = delete;
    Font& operator=(const Font&) = delete;
    // movable
    Font(Font&&) = default;
};

inline HFONT get_window_font(HWND hwnd) {
    return reinterpret_cast<HFONT>(SendMessage(hwnd, WM_GETFONT, 0, 0));
}

// Christopher Janzon https://stackoverflow.com/a/17075471/18396947
inline bool CALLBACK set_window_font_callback(HWND hwnd, LPARAM lp) {
    SendMessage(hwnd, WM_SETFONT, lp, true);
    return true;
}

inline void set_window_font(HWND hwnd, HFONT font) {
    EnumChildWindows
        (hwnd, reinterpret_cast<WNDENUMPROC>(set_window_font_callback),
         reinterpret_cast<LPARAM>(font));
}

inline void paint_text
(HDC hdc, const std::wstring& text, COLORREF foreground, RECT* rect,
 HFONT font=NULL, int flags=0) {
    HFONT prev_fn = NULL;
    if (font) prev_fn = static_cast<HFONT>(SelectObject(hdc, font));
    auto prev_bk = SetBkMode(hdc, TRANSPARENT);
    auto prev_cl = SetTextColor(hdc, foreground);
    DrawTextW(hdc, text.data(), static_cast<int>(text.size()), rect, flags);
    SetBkMode(hdc, prev_bk);
    SetTextColor(hdc, prev_cl);
    if (font) SelectObject(hdc, prev_fn);
}

inline RECT get_text_rect
(HDC hdc, const std::wstring& text, HFONT font=NULL) {
    HFONT prev_fn = NULL;
    if (font) prev_fn = static_cast<HFONT>(SelectObject(hdc, font));
    RECT rect {};
    DrawTextW(hdc, text.data(), static_cast<int>(text.size()), &rect,
              DT_CALCRECT | DT_NOPREFIX | DT_SINGLELINE);
    if (font) SelectObject(hdc, prev_fn);
    return rect;
}

inline int get_text_width
(HDC hdc, const std::wstring& text, HFONT font=NULL) {
    auto rect = get_text_rect(hdc, text, font);
    return rect.right - rect.left;
}

inline int get_text_height
(HDC hdc, const std::wstring& text, HFONT font=NULL) {
    auto rect = get_text_rect(hdc, text, font);
    return rect.bottom - rect.top;
}

inline RECT get_centred_text_rect
(HDC hdc, const std::wstring& text, RECT* container, HFONT font=NULL,
 bool horizontally=true, bool vertically=true, bool bottom=false,
 bool right=false) {
    auto rect = get_text_rect(hdc, text, font);
    OffsetRect(&rect, container->left, container->top);
    auto w = rect.right - rect.left;
    auto h = rect.bottom - rect.top;
    auto cw = container->right - container->left;
    auto ch = container->bottom - container->top;
    if (horizontally) OffsetRect(&rect, cw/2 - w/2, 0);
    else if (right) OffsetRect(&rect, cw - w, 0);
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

// https://learn.microsoft.com/en-us/windows/win32/gdi/drawing-a-shaded-rectangle
inline void paint_gradient_rect
(HDC hdc, POINT p1, POINT p2, COLORREF c1, COLORREF c2) {
    // Create an array of TRIVERTEX structures that describe
    // positional and color values for each vertex. For a rectangle,
    // only two vertices need to be defined: upper-left and lower-right.
    TRIVERTEX vertex[2] {};
    vertex[0].x     = p1.x;
    vertex[0].y     = p1.y;
    vertex[0].Red   = MAKEWORD(0, GetRValue(c1));
    vertex[0].Green = MAKEWORD(0, GetGValue(c1));
    vertex[0].Blue  = MAKEWORD(0, GetBValue(c1));
    vertex[0].Alpha = 0x0000;

    vertex[1].x     = p2.x;
    vertex[1].y     = p2.y;
    vertex[1].Red   = MAKEWORD(0, GetRValue(c2));
    vertex[1].Green = MAKEWORD(0, GetGValue(c2));
    vertex[1].Blue  = MAKEWORD(0, GetBValue(c2));
    vertex[1].Alpha = 0x0000;

    // Create a GRADIENT_RECT structure that
    // references the TRIVERTEX vertices.
    GRADIENT_RECT gRect {};
    gRect.UpperLeft  = 0;
    gRect.LowerRight = 1;

    // Draw a shaded rectangle.
    GradientFill(hdc, vertex, 2, &gRect, 1, GRADIENT_FILL_RECT_H);
}
