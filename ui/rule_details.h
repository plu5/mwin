#pragma once

#include <string> // std::string, std::wstring
#include <Windows.h> // HWND, HINSTANCE, WPARAM, LPARAM, DWORD
#include <vector> // std::vector
#include "core/rules.h" // Rule, RuleFieldType, RuleFieldChange
#include "ui/edit.h" // Edit
#include "ui/base.h" // Window, create_window
#include "utility/win32_painting.h" // CompatDc, CompatBitmap
#include "constants.h" // Theme

struct RuleField {
    RuleFieldType type;
    Edit* edit;
    std::string label;
    int x, y;
};

class RuleDetails : public Window {
    using super = Window;
public:
    HWND hwnd = NULL;
    RuleDetails
    (std::wstring title, std::wstring class_name, HINSTANCE hinst)
        : Window(title, class_name, hinst) {};
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
    int marg = 5;
    int edit_height = 20;
    std::vector<RuleField> fields = {
        {RuleFieldType::name, &rule_name_edit, "Rule name:",
         marg, marg},
        {RuleFieldType::commentary, &commentary_edit, "Commentary:",
         marg, edit_height + 2*marg},
        {RuleFieldType::wnd_title, &wnd_title_edit, "Window title:",
         marg, edit_height*3 + 4*marg},
        {RuleFieldType::wnd_exe, &wnd_exe_edit, "Exe path:",
         marg, edit_height*4 + 5*marg}
    };
    HINSTANCE hinst = NULL;
    bool events_enabled = false;
    void enable_events();
    void disable_events();
    LRESULT proc(UINT msg, WPARAM wp, LPARAM lp) override;
    // Positioning
    int y = 0;
    // TODO(plu5): Calculate dynamically with adjustments depending on the
    // actual size of things and font sizes which may alter layout
    int useful_height = 70; // Full height for scrolling
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
    void paint_selectors_header(HDC hdc);
    int label_foreground = Theme::fg;
    int separator_height = 2;
    int separator_label_left_offset = 20;
    std::wstring selectors_label = L"SELECTORS";
    Font separator_font;
};
