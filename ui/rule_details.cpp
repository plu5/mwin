#include "ui/rule_details.h"
#include <commctrl.h> // WC_EDIT
#include <windowsx.h> // Edit_SetText, Edit_GetTextLength, Edit_GetText
#include "plog/Log.h"
#include "utility/win32_casts.h" // hmenu_cast
#include "utility/win32_geometry.h" // get_size
#include "utility/string_conversion.h" // string_to_wstring, wstring_to_string
#include "core/coords.h" // WndCoordinates

void RuleDetails::initialise(HWND parent_hwnd_, int y_) {
    parent_hwnd = parent_hwnd_;
    y = y_;

    auto size = get_size(parent_hwnd);
    WndCoordinates geom = {0, y, size.w, size.h - y};
    hwnd = create_window<RuleDetails>(*this, hinst, &geom, background, WS_CHILDWINDOW | WS_VISIBLE, parent_hwnd);

    // NOTE(plu5): For correct scrollbar at launch, have to have this here;
    // doesn't work in WM_CREATE. And have to do this before children get
    // initialised in order that they get correct geometry.
    adjust_scrollinfo(get_size(hwnd).h);

    setup_paint_buffers();

    rule_name_edit.initialise
        (hwnd, hinst, marg, marg,
         get_size(hwnd).w - marg*2, 20, "Rule name:");
    commentary_edit.initialise
        (hwnd, hinst, marg, 20 + 2*marg,
         get_size(hwnd).w - marg*2, 20, "Commentary:");
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
    hdc = GetDC(hwnd);
    auto size = get_size(hwnd);
    bmp.initialise(hdc, size.w, size.h);
    dc2.initialise(hdc, hwnd);
    hdc2 = dc2.hdc;
    SelectObject(hdc2, bmp.hb);
    ReleaseDC(hwnd, hdc);
}

void RuleDetails::paint() {
    PAINTSTRUCT ps;
    auto size = get_size(hwnd);
    FillRect(hdc2, &size.rect, background);

    for (auto* edit : edits) edit->paint(hdc2);

    hdc = BeginPaint(hwnd, &ps);
    BitBlt(hdc, 0, 0, size.w, size.h, hdc2, 0, 0, SRCCOPY);
    EndPaint(hwnd, &ps);
}
