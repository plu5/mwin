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

    for (auto& field : fields) {
        WndCoordinates fgeom = calculate_field_geometry(field);
        field.edit->initialise
            (hwnd, hinst, fgeom.x, fgeom.y, fgeom.w, fgeom.h,
             field.label, label_foreground, field.label_width);
    }
}

WndCoordinates RuleDetails::calculate_field_geometry(RuleField field) {
    if (field.type == RuleFieldType::coords) {
        auto w = (get_size(hwnd).w - 5*marg) / 4;
        auto i = field.horizontal_pos;
        return {(i+1)*marg + i*w, field.y, w, edit_height};
    } else {
        return {field.x, field.y, get_size(hwnd).w - marg*2, edit_height};
    }
}

void RuleDetails::adjust_size() {
    auto size = get_size(parent_hwnd);
    SetWindowPos(hwnd, NULL, 0, 0, size.w, size.h - y, SWP_NOMOVE);
    for (auto& field : fields) {
        auto fgeom = calculate_field_geometry(field);
        field.edit->resize_width(fgeom.w);
        if (field.x == dynamic or field.y == dynamic)
            field.edit->reposition(fgeom.x, fgeom.y); 
    }
}

void RuleDetails::enable_events() {
    events_enabled = true;
}

void RuleDetails::disable_events() {
    events_enabled = false;
}

void RuleDetails::populate(const Rule& rule) {
    disable_events();
    auto i = 0;
    for (auto& field : fields) {
        if (field.type == RuleFieldType::coords) {
            field.edit->populate
                (std::to_string(rule.get(field.type).coords[i]));
            i += 1;
        } else {
            field.edit->populate(rule.get(field.type).str);
        }
    }
    enable_events();
}

void RuleDetails::clear_and_disable() {
    disable_events();
    for (auto& field : fields) field.edit->clear_and_disable();
    enable_events();
}

WndCoordinates RuleDetails::get_coords() {
    WndCoordinates coords {};
    auto i = 0;
    for (auto& edit : {x_edit, y_edit, w_edit, h_edit}) {
        try {
            coords[i] = std::stoi(edit.text());
        } catch (std::invalid_argument&) { // leave as 0
        } catch (std::out_of_range&) {     // leave as 0
        }
        i += 1;
    }
    return coords;
}

RuleFieldChange RuleDetails::command(WPARAM wp, LPARAM lp) {
    if (!events_enabled) return {};
    auto hwnd_ = reinterpret_cast<HWND>(lp);
    if (HIWORD(wp) == EN_CHANGE) {
        for (auto& field : fields) {
            if (hwnd_ == field.edit->hwnd) {
                if (field.type == RuleFieldType::coords) {
                    return {field.type, {.coords = get_coords()}}; 
                } else {
                    return {field.type, {.str = field.edit->text()}};
                }
            }
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
    dc2.select_bitmap(bmp.h);
    ReleaseDC(hwnd, hdc1);
}

void RuleDetails::paint() {
    PAINTSTRUCT ps;
    auto size = get_size(hwnd);
    paint_rect(dc2.h, &size.rect, Theme::bg);

    for (auto& field : fields) field.edit->paint(dc2.h);
    paint_section_header(dc2.h, 3, selectors_label);
    paint_section_header(dc2.h, 5, geometry_label);

    hdc1 = BeginPaint(hwnd, &ps);
    BitBlt(hdc1, 0, 0, size.w, size.h, dc2.h, 0, 0, SRCCOPY);
    EndPaint(hwnd, &ps);
}

void RuleDetails::paint_section_header
(HDC hdc, size_t vertical_order_position, const std::wstring& label) {
    auto pos = vertical_order_position - 2;
    if (pos < 0 or pos >= fields.size()) pos = 0;
    auto& last_edit = *fields.at(pos).edit;
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
        get_text_width(hdc, label, &separator_font);

    paint_rect(hdc, &rect, Theme::fg);
    paint_rect(hdc, &text_rect, Theme::bg);

    paint_text(hdc, label, Theme::fg, &text_rect, &separator_font);
}
