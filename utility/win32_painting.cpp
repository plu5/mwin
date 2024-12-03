#include "utility/win32_painting.h"

void CompatDc::initialise(HDC hdc) {
    delete_if_initialised();
    h = CreateCompatibleDC(hdc);
}

void CompatDc::delete_if_initialised() {
    if (old_bmp and old_bmp_dc) {
        SelectObject(old_bmp_dc, old_bmp);
        old_bmp = NULL;
        old_bmp_dc = NULL;
    }
    // A DC obtained via GetDC should be released with ReleaseDC, but a DC
    // created ourselves should be deleted, otherwise it will leak.
    if (h) DeleteDC(h);
}

void CompatDc::select_bitmap(HBITMAP bmp) {
    old_bmp = static_cast<HBITMAP>(SelectObject(h, bmp));
    old_bmp_dc = h;
}

CompatDc::CompatDc(HDC hdc) {
    initialise(hdc);
}

CompatDc::~CompatDc() {
    delete_if_initialised();
}

void CompatDc::clear_state() {
    h = NULL;
    old_bmp = NULL;
    old_bmp_dc = NULL;
}

// Can't use std::exchange because NULL doesn't go in win32 object handles
// easily (nor does INVALID_HANDLE_VALUE)
CompatDc::CompatDc(CompatDc&& other) noexcept
    : h(std::move(other.h)), old_bmp(std::move(other.old_bmp)),
      old_bmp_dc(std::move(other.old_bmp_dc)){
    other.clear_state();
}

void CompatBitmap::initialise(HDC hdc, int width, int height) {
    delete_if_initialised();
    p.reset(CreateCompatibleBitmap(hdc, width, height));
}

CompatBitmap::CompatBitmap(HDC hdc, int width, int height) {
    initialise(hdc, width, height);
}

void Icon::initialise(HINSTANCE hinst, int id) {
    h = LoadIcon(hinst, MAKEINTRESOURCE(id));
}

void Font::initialise(LOGFONT logfont) {
    delete_if_initialised();
    h = CreateFontIndirect(&logfont);
}

void Font::initialise
(HDC hdc, const std::wstring& face, int pt, bool bold /* =false */) {
    LOGFONT logfont {};
    wcscpy_s(logfont.lfFaceName, face.data());
    logfont.lfHeight = -MulDiv(pt, GetDeviceCaps(hdc, LOGPIXELSY), 72);
    if (bold) logfont.lfWeight = FW_BOLD;
    initialise(logfont);
}

void Font::from_current
(HDC hdc, bool italic /* =false */, bool bold /* =false */) {
    HFONT current = static_cast<HFONT>(GetCurrentObject(hdc, OBJ_FONT));
    LOGFONT logfont {};
    GetObject(current, sizeof(LOGFONT), &logfont);
    if (italic) logfont.lfItalic = true;
    if (bold) logfont.lfWeight = FW_BOLD;
    initialise(logfont);
}

void Font::from_resource
(HDC hdc, int id, const std::wstring& face, int pt, bool bold /* =false */,
 const std::wstring& fallback /* =L"MS Dlg 2" */) {
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

void Font::delete_if_initialised(bool res /* =false */) {
    if (h) DeleteObject(h);
    if (res and res_h) RemoveFontMemResourceEx(res_h);
}

void Font::clear_state() {
    h = NULL;
    res_h = NULL;
}

Font::Font(Font&& other) noexcept
    : h(std::move(other.h)), res_h(std::move(other.res_h)) {
    other.clear_state();
}
