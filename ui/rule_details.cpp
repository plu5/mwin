#include "ui/rule_details.h"
#include <windowsx.h> // Edit_SetText, Edit_GetTextLength, Edit_GetText
#include "plog/Log.h"
#include "utility/win32_casts.h" // hmenu_cast
#include "utility/win32_geometry.h" // get_size, get_rect
#include "utility/win32_painting.h" // paint_text, paint_rect
#include "utility/string_conversion.h" // string_to_wstring, wstring_to_string
#include "core/coords.h" // WndCoordinates

void RuleDetails::initialise(HWND parent_hwnd_, int y_) {
    parent_hwnd = parent_hwnd_;
    y = y_;

    auto size = get_size(parent_hwnd);
    WndCoordinates geom = {0, y, size.w, size.h - y};
    hwnd = create_window<RuleDetails>
        (*this, hinst, &geom, WS_CHILDWINDOW | WS_VISIBLE, parent_hwnd);

    // NOTE(plu5): For correct scrollbar at launch, have to have this here;
    // doesn't work in WM_CREATE. And have to do this before children get
    // initialised in order that they get correct geometry.
    adjust_scrollinfo(get_size(hwnd).h);

    setup_paint_buffers();

    rule_name_edit.initialise
        (hwnd, hinst, marg, marg,
         get_size(hwnd).w - marg*2, edit_height, "Rule name:",
         label_foreground);
    commentary_edit.initialise
        (hwnd, hinst, marg, edit_height + 2*marg,
         get_size(hwnd).w - marg*2, edit_height, "Commentary:",
         label_foreground);
}

void RuleDetails::adjust_size() {
    auto size = get_size(parent_hwnd);
    SetWindowPos(hwnd, NULL, 0, 0, size.w, size.h - y, SWP_NOMOVE);
    auto w = get_size(hwnd).w - marg*2;
    for (auto& edit : edits) edit->resize_width(w);
}

void RuleDetails::enable_events() {
    events_enabled = true;
}

void RuleDetails::disable_events() {
    events_enabled = false;
}

void RuleDetails::populate(const Rule& rule) {
    disable_events();
    rule_name_edit.populate(rule.name);
    commentary_edit.populate(rule.commentary);
    enable_events();
}

void RuleDetails::clear_and_disable() {
    disable_events();
    for (auto* edit : edits) edit->clear_and_disable();
    enable_events();
}

RuleFieldChange RuleDetails::command(WPARAM wp, LPARAM lp) {
    if (!events_enabled) return {};
    auto hwnd_ = reinterpret_cast<HWND>(lp);
    if (HIWORD(wp) == EN_CHANGE) {
        if (hwnd_ == rule_name_edit.hwnd) {
            return {RuleField::name, {rule_name_edit.text()}}; 
        } else if (hwnd_ == commentary_edit.hwnd) {
            return {RuleField::commentary, {commentary_edit.text()}}; 
        }
    }
    return {};
}

// n is height if `range' is true, pos otherwise
void RuleDetails::adjust_scrollinfo(int n, bool range) {
    SCROLLINFO si = {0};
    si.cbSize = sizeof(si);
    if (range) { // adjust range
        si.fMask = SIF_RANGE | SIF_PAGE;
        si.nMin = 0;
        si.nMax = useful_height;
        si.nPage = n;
    } else {     // adjust pos
        si.fMask = SIF_POS;
        si.nPos = n;
        si.nTrackPos = 0;
    }
    SetScrollInfo(hwnd, SB_VERT, &si, true);
    // Scroll to compensate for new position if necessary
    si.fMask = SIF_POS;
    GetScrollInfo(hwnd, SB_VERT, &si);
    auto pos = si.nPos;
    if (pos != scroll_y) {
        ScrollWindow(hwnd, 0, scroll_y - pos, NULL, NULL);
        scroll_y = pos;
    }
}

LRESULT RuleDetails::proc(UINT msg, WPARAM wp, LPARAM lp) {
    switch (msg) {
    case WM_CREATE:
    case WM_SETTINGCHANGE:
        // Calculate scroll delta
        SystemParametersInfo(SPI_GETWHEELSCROLLLINES, 0, &wheel_scroll_lines, 0);
        if (wheel_scroll_lines)
            scroll_delta_per_line = WHEEL_DELTA / wheel_scroll_lines;
        else
            scroll_delta_per_line = 0;
        break;

    case WM_SIZE:
        adjust_scrollinfo(HIWORD(lp));
        setup_paint_buffers();
        break;

    case WM_VSCROLL:
        vscroll(wp);
        break;

    case WM_MOUSEWHEEL:
        vscroll_by_mousewheel(wp);
        break;

    case WM_COMMAND:
        PostMessage(parent_hwnd, msg, wp, lp);
        break;

    case WM_PAINT:
        paint();
        break;
    }
    return super::proc(msg, wp, lp);
}

// Petzold
void RuleDetails::vscroll_by_mousewheel(WPARAM wp) {
    if (scroll_delta_per_line == 0) return;
    scroll_accumulated_delta += (short)HIWORD(wp); // 120 or -120
    while (scroll_accumulated_delta >= scroll_delta_per_line) {
        SendMessage(hwnd, WM_VSCROLL, SB_LINEUP, 0);
        scroll_accumulated_delta -= scroll_delta_per_line;
    }
    while (scroll_accumulated_delta <= -scroll_delta_per_line) {
        SendMessage(hwnd, WM_VSCROLL, SB_LINEDOWN, 0);
        scroll_accumulated_delta += scroll_delta_per_line;
    }
}

// Jichao https://stackoverflow.com/a/32096158/18396947
void RuleDetails::vscroll(WPARAM wp) {
    auto action = LOWORD(wp);
    int pos = -1;
    if (action == SB_THUMBPOSITION || action == SB_THUMBTRACK) {
        pos = HIWORD(wp);
    } else if (action == SB_LINEDOWN) {
        pos = scroll_y + scroll_delta_per_line;
    } else if (action == SB_LINEUP) {
        pos = scroll_y - scroll_delta_per_line;
    } 
    if (pos == -1) return;
    adjust_scrollinfo(pos, false);
}

void RuleDetails::setup_paint_buffers() {
    hdc1 = GetDC(hwnd);
    auto size = get_size(hwnd);
    bmp.initialise(hdc1, size.w, size.h);
    dc2.initialise(hdc1, hwnd);
    hdc2 = dc2.handle;
    SelectObject(hdc2, bmp.handle);
    ReleaseDC(hwnd, hdc1);
}

void RuleDetails::paint() {
    PAINTSTRUCT ps;
    auto size = get_size(hwnd);
    paint_rect(hdc2, &size.rect, Theme::bg);

    for (auto* edit : edits) edit->paint(hdc2);
    paint_selectors_header(hdc2);

    hdc1 = BeginPaint(hwnd, &ps);
    BitBlt(hdc1, 0, 0, size.w, size.h, hdc2, 0, 0, SRCCOPY);
    EndPaint(hwnd, &ps);
}

void RuleDetails::paint_selectors_header(HDC hdc) {
    auto& last_edit = *edits.back();
    auto rect = get_relative_rect(last_edit.hwnd, hwnd);
    auto move_down = (rect.bottom - rect.top) + last_edit.label_top_offset + marg;
    rect.top += move_down;
    rect.bottom += move_down;
    rect.left = marg;
    rect.right = get_size(hwnd).w - marg;
    auto text_rect = rect;
    text_rect.left += separator_label_left_offset;

    TEXTMETRIC tm {};
    GetTextMetrics(hdc, &tm);
    auto text_height = tm.tmHeight + tm.tmExternalLeading;
    rect.top += text_height/2;
    rect.bottom = rect.top + separator_height;

    if (not separator_font.initialised) separator_font.from_current(hdc, true);
    text_rect.right = text_rect.left +
        get_text_width(hdc, selectors_label, &separator_font);

    paint_rect(hdc, &rect, Theme::fg);
    paint_rect(hdc, &text_rect, Theme::bg);

    paint_text(hdc, selectors_label, Theme::fg, &text_rect, &separator_font);
}
