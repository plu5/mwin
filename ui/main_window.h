#pragma once

#include "ui/base.h"
#include "core/config.h"
#include "ui/rules_list.h"

class MainWindow : public Window {
public:
    using super = Window;
    MainWindow
    (std::wstring title, std::wstring class_name, HINSTANCE hinst, int show)
        : Window(title, class_name, hinst, show), rules_list(hwnd, hinst) {};
protected:
    Config config;
    RulesList rules_list;
    LRESULT proc(UINT msg, WPARAM wp, LPARAM lp) override;
    static INT_PTR CALLBACK s_about_proc
    (HWND hwnd, UINT msg, WPARAM wp, LPARAM);
    void initialise();
    void finalise();
    void update_geometry();
    void save_geometry();
};
