#pragma once

#include <string> // std::string, std::wstring
#include <Windows.h> // HWND, HINSTANCE, WPARAM, LPARAM, DWORD
#include <vector> // std::vector
#include "core/rules.h" // Rule
#include "ui/edit.h" // Edit
#include "ui/base.h" // Window, create_window
#include "utility/win32_painting.h" // CompatDc, CompatBitmap
#include "constants.h" // Theme

enum class RuleField {none, name, commentary};

struct RuleFieldData {
    std::string str;
};

struct RuleFieldChange {
    RuleField field = RuleField::none;
    RuleFieldData data;
};

class RuleDetails : public Window {
    using super = Window;
public:
    HWND hwnd = 0;
    RuleDetails
    (std::wstring title, std::wstring class_name, HINSTANCE hinst)
        : Window(title, class_name, hinst) {};
    void initialise(HWND parent_hwnd_, int y_);
    void adjust_size();
    void populate(const Rule& rule);
    void clear_and_disable();
    RuleFieldChange command(WPARAM wp, LPARAM lp);
protected:
    HWND parent_hwnd = 0;
    Edit rule_name_edit;
    Edit commentary_edit;
    std::vector<Edit*> edits = {&rule_name_edit, &commentary_edit};
    HINSTANCE hinst = 0;
    bool events_enabled = false;
    void enable_events();
    void disable_events();
    LRESULT proc(UINT msg, WPARAM wp, LPARAM lp) override;
    // Positioning
    int y = 0;
    int marg = 5;
    // TODO(plu5): Calculate dynamically with adjustments depending on the
    // actual size of things and font sizes which may alter layout
    int useful_height = 70; // Full height for scrolling
    int edit_height = 20;
    // Scrolling logic
    int scroll_y = 0, scroll_delta_per_line = 30, scroll_accumulated_delta = 0,
        wheel_scroll_lines = 0;
    void adjust_scrollinfo(int height, bool range=true);
    void vscroll_by_mousewheel(WPARAM wp);
    void vscroll(WPARAM wp);
    // Painting & double-buffering
    HDC hdc1 = 0;
    HDC hdc2 = 0;
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
