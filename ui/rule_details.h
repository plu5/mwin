#pragma once

#include <string> // std::string, std::wstring
#include <Windows.h> // HWND, HINSTANCE, WPARAM, LPARAM, DWORD
#include <vector> // std::vector
#include "core/rules.h" // Rule, RuleFieldType, RuleFieldChange
#include "ui/edit.h" // Edit
#include "ui/select.h" // Select
#include "ui/base.h" // Window, create_window
#include "ui/grab_dialog.h" // GrabDialog
#include "ui/identify_indicator.h" // IdentifyIndicator
#include "ui/tristate.h" // Tristate
#include "utility/win32_painting.h" // CompatDc, CompatBitmap, Icon
#include "constants.h" // Theme

struct RuleField {
    RuleFieldType type = RuleFieldType::none;
    Edit* edit = nullptr;
    Select* select = nullptr;
    Tristate* tristate = nullptr;
    std::string label = "";
    int x = 0, y = 0, label_width = 90, horizontal_pos = 0;
};

class RuleDetails : public Window {
    using super = Window;
public:
    HWND hwnd = NULL;
    RuleDetails
    (std::wstring title, std::wstring class_name, HINSTANCE hinst)
        : Window(title, class_name, hinst), grab_icon(hinst, IDI_PICKER),
          trigger_icon(hinst, IDI_PLAY), edits_bg(Theme::edits_bg),
          identify_indicator
          (L"Identify monitor", L"mwinIdentifyIndicator", hinst),
          grab_dialog(L"Grab dialog", L"mwinGrabDialog", hinst)
    {};
    void initialise(HWND parent_hwnd_, int y_);
    void adjust_size();
    void populate(const Rule& rule);
    void clear_and_disable();
    RuleFieldChange command(WPARAM wp, LPARAM lp);
    void post_grab();
protected:
    HWND parent_hwnd = NULL;
    Brush edits_bg;
    Edit rule_name_edit;
    Edit commentary_edit;
    Edit wnd_title_edit;
    Edit wnd_exe_edit;
    Edit x_edit;
    Edit y_edit;
    Edit w_edit;
    Edit h_edit;
    HWND grab_btn = NULL;
    int grab_btn_size = 21;
    Icon grab_icon;
    HWND trigger_btn = NULL;
    Icon trigger_icon;
    GrabDialog grab_dialog;
    Select monitor_select;
    HWND identify_monitor_btn = NULL;
    std::wstring identify_monitor_text = L"This is monitor ";
    IdentifyIndicator identify_indicator;
    Tristate borderless_tristate;
    Tristate alwaysontop_tristate;
    int marg = 5;
    int edit_height = 20;
    int btn_size = 25;
    int dynamic = -1;
    std::vector<RuleField> fields = {
        {.type = RuleFieldType::name, .edit = &rule_name_edit,
         .label = "Rule name:", .x = marg, .y = grab_btn_size + marg},
        {.type = RuleFieldType::commentary, .edit = &commentary_edit,
         .label = "Commentary:",
         .x = marg, .y = grab_btn_size + edit_height + 2*marg},
        {.type = RuleFieldType::wnd_title, .edit = &wnd_title_edit,
         .label = "Window title:",
         .x = marg, .y = grab_btn_size + edit_height*3 + 4*marg},
        {.type = RuleFieldType::wnd_exe, .edit = &wnd_exe_edit,
         .label = "Exe path:", .x = marg,
         .y = grab_btn_size + edit_height*4 + 5*marg},
        {.type = RuleFieldType::monitor, .select = &monitor_select,
         .x = marg + btn_size, .y = grab_btn_size + edit_height*6 + 7*marg},
        {.type = RuleFieldType::coords, .edit = &x_edit, .label = "X:",
         .x = dynamic, .y = grab_btn_size + edit_height*7 + 9*marg,
         .label_width = 18, .horizontal_pos = 0},
        {.type = RuleFieldType::coords, .edit = &y_edit, .label = "Y:",
         .x = dynamic, .y = grab_btn_size + edit_height*7 + 9*marg,
         .label_width = 18, .horizontal_pos = 1},
        {.type = RuleFieldType::coords, .edit = &w_edit, .label = "W:",
         .x = dynamic, .y = grab_btn_size + edit_height*7 + 9*marg,
         .label_width = 18, .horizontal_pos = 2},
        {.type = RuleFieldType::coords, .edit = &h_edit, .label = "H:",
         .x = dynamic, .y = grab_btn_size + edit_height*7 + 9*marg,
         .label_width = 18, .horizontal_pos = 3},
        {.type = RuleFieldType::borderless, .tristate = &borderless_tristate,
         .label = "Borderless:",
         .x = marg, .y = grab_btn_size + edit_height*9 + 14*marg},
        {.type = RuleFieldType::alwaysontop, .tristate = &alwaysontop_tristate,
         .label = "AlwaysOnTop:",
         .x = marg, .y = grab_btn_size + edit_height*10 + 16*marg},
    };
    bool events_enabled = false;
    const Rule* current_rule = nullptr;
    void enable_events();
    void disable_events();
    void trigger();
    LRESULT proc(UINT msg, WPARAM wp, LPARAM lp) override;
    void show_grab_dialog();
    WndCoordinates get_coords();
    void set_coords(const WndCoordinates& coords);
    void populate_monitor_select();
    void set_coords_by_selected_monitor(int selected);
    void set_identify_btn_state_by_selected_monitor(int selected);
    void show_identify_indicator();
    void change_monitor_select_if_coords_differ(const WndCoordinates& coords);
    // Positioning
    int y = 0;
    int useful_height = 500; // Full height for scrolling. Adjusts dynamically
    WndCoordinates calculate_field_geometry(RuleField field);
    // Scrolling logic
    int scroll_y = 0, scroll_delta_per_line = 30, scroll_accumulated_delta = 0,
        wheel_scroll_lines = 0;
    void adjust_scrollinfo(int height, bool range=true);
    void vscroll_by_mousewheel(WPARAM wp);
    void vscroll(WPARAM wp);
    // Painting
    void paint() override;
    void paint_section_header
    (HDC hdc, size_t vertical_order_position, const std::wstring& label);
    int label_foreground = Theme::fg;
    int separator_x = 0; // dynamic
    std::wstring selectors_label = L"SELECTORS";
    std::wstring geometry_label = L"GEOMETRY";
    std::wstring modifiers_label = L"MODIFIERS";
    Font font;
    void paint_modifier_tick_labels(HDC hdc);
    std::wstring left_tick_label = L"Apply";
    std::wstring mid_tick_label = L"As is";
    std::wstring right_tick_label = L"Unapply";
    int modifier_tick_labels_y_offset = 16;
};
