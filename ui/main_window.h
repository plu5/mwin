#pragma once

#include "ui/base.h"
#include "core/config.h" // Config
#include "ui/rules_list.h" // RulesList
#include "ui/rule_details.h" // RuleDetails
#include <Windows.h> // HINSTANCE, HDC, HBITMAP, LRESULT, HWND, UINT, ...

class MainWindow : public Window {
public:
    using super = Window;
    MainWindow
    (std::wstring title, std::wstring class_name, HINSTANCE hinst, int show)
        : Window(title, class_name, hinst, show) {};
protected:
    Config config;
    RulesList rules_list;
    RuleDetails rule_details;
    LRESULT proc(UINT msg, WPARAM wp, LPARAM lp) override;
    static INT_PTR CALLBACK s_about_proc
    (HWND hwnd, UINT msg, WPARAM wp, LPARAM);
    void initialise();
    void finalise();
    void update_geometry();
    void save_geometry();
    void notify(LPARAM lp);
    void parse_menu_selections(WORD id);
    void command(WPARAM wp, LPARAM lp);
};
