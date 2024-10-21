#pragma once

#include <string> // std::string, std::wstring
#include <Windows.h> // HWND, HINSTANCE, WPARAM, LPARAM, DWORD
#include <vector> // std::vector
#include "core/rules.h" // Rule, RuleFieldType, RuleFieldChange
#include "ui/edit.h" // Edit
#include "ui/select.h" // Select
#include "ui/base.h" // Window, create_window
#include "ui/identify_indicator.h" // IdentifyIndicator
#include "utility/win32_painting.h" // CompatDc, CompatBitmap
#include "constants.h" // Theme

struct RuleField {
    RuleFieldType type = RuleFieldType::none;
    Edit* edit = nullptr;
    Select* select = nullptr;
    std::string label = "";
    int x = 0, y = 0, label_width = 90, horizontal_pos = 0;
};

class RuleDetails : public Window {
    using super = Window;
public:
    HWND hwnd = NULL;
    RuleDetails
    (std::wstring title, std::wstring class_name, HINSTANCE hinst)
        : Window(title, class_name, hinst),
          identify_indicator(L"Identify monitor", L"IdentifyIndicator",
                             hinst) {};
    void initialise(HWND parent_hwnd_, int y_);
    void adjust_size();
    void populate(const Rule& rule);
    void clear_and_disable();
    RuleFieldChange command(WPARAM wp, LPARAM lp);
protected:
    HWND parent_hwnd = NULL;
    Edit rule_name_edit;
    Edit commentary_edit;
    Edit wnd_title_edit;
    Edit wnd_exe_edit;
    Edit x_edit;
    Edit y_edit;
    Edit w_edit;
    Edit h_edit;
    Select monitor_select;
    HWND identify_monitor_btn = NULL;
    std::wstring identify_monitor_text = L"This is monitor ";
    IdentifyIndicator identify_indicator;
    int marg = 5;
    int edit_height = 20;
    int btn_size = 25;
    int dynamic = -1;
    std::vector<RuleField> fields = {
        {.type = RuleFieldType::name, .edit = &rule_name_edit,
         .label = "Rule name:", .x = marg, .y = marg},
        {.type = RuleFieldType::commentary, .edit = &commentary_edit,
         .label = "Commentary:", .x = marg, .y = edit_height + 2*marg},
        {.type = RuleFieldType::wnd_title, .edit = &wnd_title_edit,
         .label = "Window title:", .x = marg, .y = edit_height*3 + 4*marg},
        {.type = RuleFieldType::wnd_exe, .edit = &wnd_exe_edit,
         .label = "Exe path:", .x = marg, .y = edit_height*4 + 5*marg},
        {.type = RuleFieldType::monitor, .select = &monitor_select,
         .x = marg + btn_size, .y = edit_height*6 + 7*marg},
        {.type = RuleFieldType::coords, .edit = &x_edit, .label = "X:",
         .x = dynamic, .y = edit_height*7 + 9*marg, .label_width = 18,
         .horizontal_pos = 0},
        {.type = RuleFieldType::coords, .edit = &y_edit, .label = "Y:",
         .x = dynamic, .y = edit_height*7 + 9*marg, .label_width = 18,
         .horizontal_pos = 1},
        {.type = RuleFieldType::coords, .edit = &w_edit, .label = "W:",
         .x = dynamic, .y = edit_height*7 + 9*marg, .label_width = 18,
         .horizontal_pos = 2},
        {.type = RuleFieldType::coords, .edit = &h_edit, .label = "H:",
         .x = dynamic, .y = edit_height*7 + 9*marg, .label_width = 18,
         .horizontal_pos = 3}
    };
    HINSTANCE hinst = NULL;
    bool events_enabled = false;
    void enable_events();
    void disable_events();
    LRESULT proc(UINT msg, WPARAM wp, LPARAM lp) override;
    WndCoordinates get_coords();
    void set_coords(const WndCoordinates& coords);
    void populate_monitor_select();
    void set_coords_by_selected_monitor(int selected);
    void set_identify_btn_state_by_selected_monitor(int selected);
    void show_identify_indicator();
    void change_monitor_select_if_coords_differ(const WndCoordinates& coords);
    // Positioning
    int y = 0;
    // TODO(plu5): Calculate dynamically with adjustments depending on the
    // actual size of things and font sizes which may alter layout
    int useful_height = 206; // Full height for scrolling
    WndCoordinates calculate_field_geometry(RuleField field);
    // Scrolling logic
    int scroll_y = 0, scroll_delta_per_line = 30, scroll_accumulated_delta = 0,
        wheel_scroll_lines = 0;
    void adjust_scrollinfo(int height, bool range=true);
    void vscroll_by_mousewheel(WPARAM wp);
    void vscroll(WPARAM wp);
    // Painting & double-buffering
    HDC hdc1 = NULL;
    CompatDc dc2;
    CompatBitmap bmp;
    void setup_paint_buffers();
    void paint();
    void paint_section_header
    (HDC hdc, size_t vertical_order_position, const std::wstring& label);
    int label_foreground = Theme::fg;
    int separator_height = 2;
    int separator_label_left_offset = 20;
    std::wstring selectors_label = L"SELECTORS";
    std::wstring geometry_label = L"GEOMETRY";
    Font separator_font;
};
