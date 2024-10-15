#pragma once

#include <string> // std::string
#include <Windows.h> // HWND, HINSTANCE, WPARAM, LPARAM, DWORD
#include "core/rules.h" // Rule
#include "ui/edit.h" // Edit

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
    void paint(HDC hdc);
protected:
    HWND parent_hwnd = 0;
    Edit rule_name_edit;
    HINSTANCE hinst = 0;
    int y = 0;
    int marg = 5;
    bool events_enabled = false;
    void enable_events();
    void disable_events();
};
