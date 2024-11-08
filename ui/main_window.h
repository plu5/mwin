#pragma once

#include "ui/base.h"
#include "core/config.h" // Config
#include "ui/rules_list.h" // RulesList
#include "ui/rule_details.h" // RuleDetails
#include <Windows.h> // HINSTANCE, HDC, HBITMAP, LRESULT, HWND, UINT, ...

class MainWindow : public Window {
    using super = Window;
public:
    MainWindow
    (std::wstring title, std::wstring class_name, HINSTANCE hinst)
        : Window(title, class_name, hinst),
          rule_details(L"RuleDetails", L"RuleDetails", hinst) {};
protected:
    bool loaded = false;
    int min_w = 275, min_h = 150;
    Config config;
    RulesList rules_list;
    RuleDetails rule_details;
    LRESULT proc(UINT msg, WPARAM wp, LPARAM lp) override;
    static INT_PTR CALLBACK s_about_proc
    (HWND hwnd, UINT msg, WPARAM wp, LPARAM);
    void initialise();
    void post_init();
    void finalise();
    void update_geometry();
    void save_geometry();
    void notify(LPARAM lp);
    void parse_menu_selections(WORD id);
    void command(WPARAM wp, LPARAM lp);
};
