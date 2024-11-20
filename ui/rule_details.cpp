#include "ui/rule_details.h"
#include <windowsx.h> // Edit_SetText, Edit_GetTextLength, Edit_GetText
#include <uxtheme.h> // SetWindowTheme
#include "plog/Log.h"
#include "utility/win32_geometry.h" // get_size, get_rect
#include "utility/win32_painting.h" // paint_text, paint_rect, ...
#include "utility/string_conversion.h" // string_to_wstring, wstring_to_string
#include "core/coords.h" // WndCoordinates
#include "utility/monitors.h" // Monitors
#include "utility/string_concat.h" // concat
#include "ui/button.h" // create_btn
#include "ui/tristate.h" // create_trackbar

void RuleDetails::initialise(HWND parent_hwnd_, int y_) {
    parent_hwnd = parent_hwnd_;
    y = y_;

    // Calculate useful height for scrolling
    WndCoordinates lgeom = calculate_field_geometry(fields.back());
    useful_height = lgeom.y + lgeom.h;

    auto size = get_size(parent_hwnd);
    WndCoordinates geom = {0, y, size.w, size.h - y};
    hwnd = create_window<RuleDetails>
        (*this, hinst, &geom,
         WS_CHILDWINDOW | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS,
         parent_hwnd, false, true);
    SetWindowTheme(hwnd, L"DarkMode_Explorer", NULL);

    // NOTE(plu5): For correct scrollbar at launch, have to have this here;
    // doesn't work in WM_CREATE. And have to do this before children get
    // initialised in order that they get correct geometry.
    adjust_scrollinfo(get_size(hwnd).h);

    for (auto& field : fields) {
        WndCoordinates fgeom = calculate_field_geometry(field);
        if (field.edit) {
            field.edit->initialise
                (hwnd, hinst, fgeom.x, fgeom.y, fgeom.w, fgeom.h,
                 field.label, field.label_width);
        } else if (field.select) {
            field.select->initialise
                (hwnd, hinst, fgeom.x, fgeom.y, fgeom.w, fgeom.h);
        } else if (field.tristate) {
            field.tristate->initialise
                (hwnd, hinst, fgeom.x, fgeom.y, fgeom.w, fgeom.h,
                 field.label, label_foreground, field.label_width, Theme::bg);
        }
    }

    identify_monitor_btn = create_btn(L"?", marg, monitor_select.y,
                                      btn_size, btn_size, -1, hwnd, hinst);

    grab_btn = create_btn
        (L"", btn_size, 0, grab_btn_size, grab_btn_size, -1,
         hwnd, hinst, true, WS_VISIBLE | BS_ICON, grab_icon.h);
    grab_dialog.initialise(parent_hwnd, label_foreground);

    trigger_btn = create_btn
        (L"", btn_size + grab_btn_size , 0, grab_btn_size, grab_btn_size, -1,
         hwnd, hinst, true, WS_VISIBLE | BS_ICON, trigger_icon.h);
    
    populate_monitor_select();

    auto hdc = GetDC(hwnd);
    font.initialise(hdc, L"MS Dlg 2", 10);
    ReleaseDC(hwnd, hdc);
    set_window_font(hwnd, font.h);
}

void RuleDetails::show_grab_dialog() {
    auto s = get_size(parent_hwnd, false);
    auto x_ = s.rect.left + s.w/2 - grab_dialog.w/2;
    auto y_ = s.rect.top + s.h/2 - grab_dialog.h/2;
    grab_dialog.show(x_, y_);
}

void RuleDetails::post_grab() {
    if (grab_dialog.data.wnd_title)
        wnd_title_edit.populate(grab_dialog.data.wnd_title.value());
    if (grab_dialog.data.wnd_exe)
        wnd_exe_edit.populate(grab_dialog.data.wnd_exe.value());
    if (grab_dialog.data.geometry)
        set_coords(grab_dialog.data.geometry.value());
}

void RuleDetails::populate_monitor_select() {
    monitor_select.add_option("Custom coordinates");
    Monitors monitors;
    for (size_t i = 0; i < monitors.rects.size(); i++) {
        std::string s = concat
            ("Monitor ", std::to_string(i+1), " fullscreen coordinates");
        monitor_select.add_option(s);
    }
    
}

void RuleDetails::set_coords_by_selected_monitor(int selected) {
    if (selected) {
        WndCoordinates coords {};
        coords.by_monitor(selected - 1);
        set_coords(coords);
    }
}

void RuleDetails::set_identify_btn_state_by_selected_monitor(int selected) {
    Button_Enable(identify_monitor_btn, selected ? true : false);
}

void RuleDetails::show_identify_indicator() {
    auto coords = get_coords();
    auto x_ = coords.x + coords.w/2 - identify_indicator.w/2;
    auto y_ = coords.y + coords.h/2 - identify_indicator.h/2;
    identify_indicator.show
        (x_, y_,
         identify_monitor_text + std::to_wstring(monitor_select.selected()));
}

void RuleDetails::change_monitor_select_if_coords_differ
(const WndCoordinates& coords) {
    auto selected = monitor_select.selected();
    if (not selected) return; // Custom coordinates
    WndCoordinates monitor_coords {};
    monitor_coords.by_monitor(selected - 1);
    if (coords != monitor_coords) {
        monitor_select.select(0);
        set_identify_btn_state_by_selected_monitor(0);
    }
}

WndCoordinates RuleDetails::calculate_field_geometry(RuleField field) {
    auto w = get_size(hwnd).w;
    auto i = field.horizontal_pos;

    if (field.tristate) {
        return {field.x, field.y, w - marg*2,
            static_cast<int>(edit_height*1.5)};
    }

    switch (field.type) {
    case RuleFieldType::coords:
        w = (w - 5*marg) / 4;
        return {(i+1)*marg + i*w, field.y - scroll_y, w, edit_height};
    case RuleFieldType::monitor:
        return {field.x, field.y, w - marg*2 - btn_size, edit_height};
    default:
        return {field.x, field.y, w - marg*2, edit_height};
    }
}

void RuleDetails::adjust_size() {
    auto size = get_size(parent_hwnd);
    SetWindowPos(hwnd, NULL, 0, 0, size.w, size.h - y, SWP_NOMOVE);
    for (auto& field : fields) {
        auto fgeom = calculate_field_geometry(field);
        if (field.edit) {
            field.edit->resize_width(fgeom.w);
            if (field.x == dynamic or field.y == dynamic)
                field.edit->reposition(fgeom.x, fgeom.y);
        } else if (field.select) {
            field.select->resize_width(fgeom.w);
        } else if (field.tristate) {
            field.tristate->resize_width(fgeom.w);
        }
    }
}

void RuleDetails::enable_events() {
    events_enabled = true;
}

void RuleDetails::disable_events() {
    events_enabled = false;
}

void RuleDetails::trigger() {
    if (!current_rule) {
        LOG_INFO << "Unable to trigger unknown rule";
    }
    grab_dialog.windows_list.wins.reposition
        (current_rule->wnd_title, current_rule->wnd_exe, current_rule->coords,
         current_rule->borderless, current_rule->alwaysontop);
}

void RuleDetails::populate(const Rule& rule) {
    disable_events();
    current_rule = &rule;
    auto i = 0;
    for (auto& field : fields) {
        if (field.type == RuleFieldType::coords) {
            field.edit->populate
                (std::to_string(rule.get(field.type).coords[i]));
            i += 1;
        } else if (field.edit) {
            field.edit->populate(rule.get(field.type).str);
        } else if (field.type == RuleFieldType::monitor) {
            auto selected = rule.get(field.type).num;
            monitor_select.select(selected);
            set_identify_btn_state_by_selected_monitor(selected);
        } else if (field.tristate) {
            field.tristate->populate(rule.get(field.type).num);
        }
    }
    enable_events();
}

void RuleDetails::clear_and_disable() {
    disable_events();
    for (auto& field : fields) {
        if (field.edit) field.edit->clear_and_disable();
        else if (field.select) field.select->clear_and_disable();
        else if (field.tristate) field.tristate->clear_and_disable();
    }
    set_identify_btn_state_by_selected_monitor(0);
    enable_events();
}

WndCoordinates RuleDetails::get_coords() {
    WndCoordinates coords {};
    auto i = 0;
    for (auto* edit : {&x_edit, &y_edit, &w_edit, &h_edit}) {
        try {
            coords[i] = std::stoi(edit->text());
        } catch (std::invalid_argument&) { // leave as 0
        } catch (std::out_of_range&) {     // leave as 0
        }
        i += 1;
    }
    return coords;
}

void RuleDetails::set_coords(const WndCoordinates& coords) {
    auto i = 0;
    for (auto* edit : {&x_edit, &y_edit, &w_edit, &h_edit}) {
        edit->populate(std::to_string(coords[i]));
        i += 1;
    }
}

RuleFieldChange RuleDetails::command(WPARAM wp, LPARAM lp) {
    if (!events_enabled) return {};
    auto hwnd_ = reinterpret_cast<HWND>(lp);
    if (HIWORD(wp) == EN_CHANGE) { // edit
        for (auto& field : fields) {
            if (field.edit and hwnd_ == field.edit->hwnd) {
                if (field.type == RuleFieldType::coords) {
                    auto coords = get_coords();
                    change_monitor_select_if_coords_differ(coords);
                    return {field.type, {.coords = coords}}; 
                } else {
                    return {field.type, {.str = field.edit->text()}};
                }
            }
        }
    } else if (HIWORD(wp) == CBN_SELCHANGE) { // combobox
        for (auto& field : fields) {
            if (field.select and hwnd_ == field.select->hwnd) {
                auto selected = field.select->selected();
                if (field.type == RuleFieldType::monitor) {
                    set_identify_btn_state_by_selected_monitor(selected);
                    set_coords_by_selected_monitor(selected);
                }
                return {field.type, {.num = selected}};
            }
        }
    } else if (HIWORD(wp) == BN_CLICKED) { // button
        if (hwnd_ == grab_btn) {
            show_grab_dialog();
        } else if (hwnd_ == trigger_btn) {
            trigger();
        } else if (hwnd_ == identify_monitor_btn) {
            show_identify_indicator();
        }
    } else if (HIWORD(wp) == WM_HSCROLL) { // trackbar or scrollbar
        switch (LOWORD(wp)) {
        case TB_BOTTOM:
        case TB_ENDTRACK:
        case TB_LINEDOWN:
        case TB_LINEUP:
        case TB_PAGEDOWN:
        case TB_PAGEUP:
        case TB_THUMBPOSITION:
        case TB_TOP:
            for (auto& field : fields) {
                if (field.tristate and hwnd_ == field.tristate->hwnd) {
                    return {field.type, {.num = field.tristate->pos()}};
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
        break;

    case WM_VSCROLL:
        // lp : If the message is sent by a standard scroll bar, this parameter
        // is NULL.
        if (not lp) vscroll(wp);
        break;

    case WM_HSCROLL:
        // tristate
        if (lp) PostMessage
                    (parent_hwnd, WM_COMMAND, MAKELONG(LOWORD(wp), msg), lp);
        break;

    case WM_MOUSEWHEEL:
        vscroll_by_mousewheel(wp);
        break;

    case WM_COMMAND:
        PostMessage(parent_hwnd, msg, wp, lp);
        break;

    case WM_CTLCOLORSTATIC:
        return reinterpret_cast<LRESULT>(bg.h);

    case WM_CTLCOLOREDIT:
    case WM_CTLCOLORLISTBOX:
        auto hdc = reinterpret_cast<HDC>(wp);
        SetBkMode(hdc, TRANSPARENT);
        SetTextColor(hdc, Theme::fg);
        SetBkColor(hdc, Theme::edits_bg);
        return reinterpret_cast<LRESULT>(edits_bg.h);
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
        if (pos < 0) pos = 0;
    } 
    if (pos == -1) return;
    adjust_scrollinfo(pos, false);
}

void RuleDetails::paint() {
    for (auto& field : fields) {
        if (field.edit) field.edit->paint(dc2.h);
        if (field.tristate) field.tristate->paint(dc2.h);
    }
    paint_section_header(dc2.h, 3, selectors_label);
    paint_section_header(dc2.h, 5, geometry_label);
    paint_section_header(dc2.h, 8, modifiers_label);
    paint_modifier_tick_labels(dc2.h);

    super::paint();
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
    rect.left = 0;
    rect.right = get_size(hwnd).w;

    // background
    paint_rect(hdc, Theme::sep_bg, &rect);

    // text
    TEXTMETRIC tm {};
    GetTextMetrics(hdc, &tm);
    auto text_height = tm.tmHeight + tm.tmExternalLeading;
    rect.top += (rect.bottom - rect.top)/2 - text_height/2;
    if (not separator_x) separator_x = marg + last_edit.label_width;
    rect.left = separator_x;
    paint_text(hdc, label, Theme::sep_fg, &rect, font.h);
}

void RuleDetails::paint_modifier_tick_labels(HDC hdc) {
    auto rect = get_relative_rect(borderless_tristate.hwnd, hwnd);
    rect.top -= modifier_tick_labels_y_offset;

    auto text_rect = rect;
    paint_text(hdc, left_tick_label, Theme::fg, &text_rect);

    text_rect = get_centred_text_rect
        (hdc, mid_tick_label, &rect, nullptr, true, false);
    paint_text(hdc, mid_tick_label, Theme::fg, &text_rect);

    text_rect = get_centred_text_rect
        (hdc, right_tick_label, &rect, nullptr, false, false, false, true);
    paint_text(hdc, right_tick_label, Theme::fg, &text_rect);
}
