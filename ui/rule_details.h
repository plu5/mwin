#pragma once

#include <string> // std::string
#include <Windows.h> // HWND, HINSTANCE, WPARAM, LPARAM, DWORD
#include "core/rules.h" // Rule

enum class RuleField {none, name};

struct RuleFieldData {
    std::string str;
};

struct RuleFieldChange {
    RuleField field = RuleField::none;
    RuleFieldData data;
};

class RuleDetails {
public:
    void initialise(HWND parent_hwnd_, HINSTANCE hinst_, int y_);
    void adjust_size();
    void populate(const Rule& rule);
    void clear_and_disable();
    RuleFieldChange command(WPARAM wp, LPARAM lp);
    DWORD get_edit_field_sel(RuleField edit);
    void set_edit_field_sel(RuleField edit, DWORD sel);
protected:
    HWND parent_hwnd = 0;
    HWND rule_name_edit = 0;
    HINSTANCE hinst = 0;
    int y = 0;
    int marg = 5;
    int label_width = 80;
    bool events_enabled = false;
    void enable_events();
    void disable_events();
};
