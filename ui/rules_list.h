#pragma once

#include <Windows.h> // HWND, HINSTANCE, WPARAM, LPARAM
#include <filesystem> // std::filesystem::path
#include "core/rules.h" // Rule, RulesModel

class RulesList {
public:
    HWND hwnd = 0;
    HWND parent_hwnd = 0;
    HWND add_btn = 0;
    HWND dup_btn = 0;
    HWND del_btn = 0;
    HINSTANCE hinst = 0;
    int btn_size = 25;
    int height = 75;
    RulesModel rules;
    void initialise(HWND parent_hwnd_, HINSTANCE hinst_);
    void load(std::filesystem::path user_dir);
    void save(std::filesystem::path user_dir);
    void repopulate();
    void select_rule(int index);
    int selected_index() const;
    Rule* selected_rule();
    void adjust_size();
    void add_rule();
    void add_rule(const Rule& rule, size_t i, bool select=false);
    void dup_rule();
    void del_rule();
    void command(WPARAM wp, LPARAM lp);
};
