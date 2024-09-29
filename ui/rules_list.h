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
    HINSTANCE hinst;
    int btn_size = 25;
    RulesModel rules;
    RulesList(HWND parent_hwnd, HINSTANCE hinst);
    void load(std::filesystem::path user_dir);
    void save(std::filesystem::path user_dir);
    void repopulate();
    void select_rule(int index);
    int selected_index() const;
    void adjust_size();
    void add_rule();
    void add_rule(const Rule& rule, size_t i);
    void dup_rule();
    void del_rule();
    void command(WPARAM wp, LPARAM lp);
};
