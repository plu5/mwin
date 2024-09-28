#pragma once

#include <Windows.h>
#include <string>
#include "core/rules.h"

class RulesList {
public:
    HWND hwnd = 0;
    HWND parent_hwnd = 0;
    HWND add_btn = 0;
    HWND dup_btn = 0;
    HWND del_btn = 0;
    HINSTANCE hinst;
    int btn_size = 25;
    RulesModel rules;
    RulesList(HWND parent_hwnd, HINSTANCE hinst);
    void select_rule(int index);
    int selected_index() const;
    void adjust_size();
    void add_rule();
    void dup_rule();
    void del_rule();
    void command(WPARAM wp, LPARAM lp);
};
