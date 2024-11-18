#include "ui/identify_indicator.h"
#include "utility/win32_geometry.h" // get_size
#include "utility/string_formatting.h" // wformat

void IdentifyIndicator::initialise() {
    WndCoordinates geom = {x, y, w, h};
    hwnd = create_window<IdentifyIndicator>
        (*this, hinst, &geom, WS_POPUP, NULL, false);
    update_closing_label();
}

void IdentifyIndicator::show(int x_, int y_, const std::wstring& text_) {
    x = x_;
    y = y_;
    text = text_;
    SetWindowPos(hwnd, NULL, x, y, 0, 0, SWP_NOSIZE);
    ShowWindow(hwnd, SW_SHOW);
    SetTimer(hwnd, timer_id, timer_interval, NULL);
}

void IdentifyIndicator::hide() {
    ShowWindow(hwnd, SW_HIDE);
}

void IdentifyIndicator::update_closing_label() {
    auto s = std::to_wstring(seconds_left);
    closing_label = wformat(closing_label_template, s);
}

LRESULT IdentifyIndicator::proc(UINT msg, WPARAM wp, LPARAM lp) {
    switch (msg) {
    case WM_PAINT:
        paint();
        break;

    case WM_TIMER:
        if (wp == timer_id) {
            if (seconds_left > 1) {
                seconds_left -= 1;
                update_closing_label();
                InvalidateRect(hwnd, NULL, FALSE);
            } else {
                KillTimer(hwnd, timer_id);
                seconds_left = initial_seconds_left;
                update_closing_label();
                hide();
            }
        }
    }
    return super::proc(msg, wp, lp);
}

void IdentifyIndicator::paint() {
    PAINTSTRUCT ps;
    auto size = get_size(hwnd);
    auto hdc = BeginPaint(hwnd, &ps);

    paint_rect(hdc, Theme::bg, &size.rect);
    auto rect = size.rect;
    rect.bottom = rect.top + top_bar_height;
    paint_rect(hdc, top_bar_colour, &rect);

    auto text_rect = get_centred_text_rect
        (hdc, closing_label, &size.rect, nullptr, true, false, true);
    text_rect.top -= closing_label_bottom_margin;
    paint_text(hdc, closing_label.data(), Theme::fg, &text_rect);

    if (not big_font.initialised)
        big_font.initialise(hdc, big_font_face, big_font_size, true);
    text_rect = get_centred_text_rect(hdc, text, &size.rect, big_font.h);
    paint_text(hdc, text.data(), Theme::fg, &text_rect, big_font.h);

    EndPaint(hwnd, &ps);
}
