#pragma once

#include <Windows.h>
#include <string> // std::wstring
#include <memory> // unique_ptr
#include <type_traits> // std::remove_pointer_t

// https://github.com/namazso/OpenHashTab/blob/
// e1511d92eac375eff91b11e9475a86d99b23fa64/OpenHashTab/utl.h#L161
struct GdiObjectDeleter {
    void operator()(HGDIOBJ hobj) const {
        if (hobj) DeleteObject(hobj);
    }
};

struct Brush {
    inline HBRUSH get() {return p.get();};
    inline Brush(int clr) : p(CreateSolidBrush(clr)) {}
private:
    std::unique_ptr<std::remove_pointer_t<HBRUSH>, GdiObjectDeleter> p;
};

struct CompatDc {
    inline HDC get() {return h;};
    void initialise(HDC hdc);
    void select_bitmap(HBITMAP bmp);
    inline CompatDc() {};
    CompatDc(HDC hdc);
    ~CompatDc();
    // Intentional lack of copy constructor, assignment, and move
    // assignment. Compiler does not define them if there is a user-defined
    // move constructor.
    CompatDc(CompatDc&& other) noexcept;
private:
    HDC h = NULL;
    HBITMAP old_bmp = NULL;
    HDC old_bmp_dc = NULL;
    void delete_if_initialised();
    void clear_state();
};

struct CompatBitmap {
    inline HBITMAP get() {return p.get();};
    void initialise(HDC hdc, int width, int height);
    inline CompatBitmap() {};
    CompatBitmap(HDC hdc, int width, int height);
private:
    std::unique_ptr<std::remove_pointer_t<HBITMAP>, GdiObjectDeleter> p;
    inline void delete_if_initialised() {if (get()) p.get_deleter()(get());}
};

// LoadIcon loads a "shared icon" which doesn't need to be deleted.
// There is also no issue with calling it several times; if already loaded, it
// retrieves a handle to the existing resource.
// So there is not really a need to wrap it but I already did so nvm...
struct Icon {
    inline HICON get() {return h;};
    void initialise(HINSTANCE hinst, int id);
    inline Icon() {}
    inline Icon(HINSTANCE hinst, int id) {initialise(hinst, id);}
private:
    HICON h = NULL;
};

struct Font {
    inline HFONT get() {return h;};
    inline bool initialised() {return h;};
    void initialise(LOGFONT logfont);
    void initialise
    (HDC hdc, const std::wstring& face, int pt, bool bold=false);
    void from_current(HDC hdc, bool italic=false, bool bold=false);
    void from_resource
    (HDC hdc, int id, const std::wstring& face, int pt, bool bold=false,
     const std::wstring& fallback=L"MS Dlg 2");
    inline Font() {};
    inline ~Font() {delete_if_initialised(true);}
    // Intentional lack of copy constructor, assignment, and move
    // assignment. Compiler does not define them if there is a user-defined
    // move constructor.
    Font(Font&& other) noexcept;
private:
    HFONT h = NULL;
    HANDLE res_h = NULL;
    void delete_if_initialised(bool res=false);
    void clear_state();
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
