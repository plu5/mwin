#pragma once

#include <Windows.h>
#include <set> // std::set
#include <string> // std::string
#include "core/coords.h" // WndCoordinates

struct OpenWindow {
    std::string title = "";
    std::string path = "";
    HWND hwnd;
};

struct OpenWindows {
    std::vector<OpenWindow> list;
    std::set<std::string> exclusions = {
        "Program Manager", "Microsoft Text Input Application"
    };

    bool should_exclude(const std::string& str);
    std::string get_title(HWND hwnd);
    std::string get_path(HWND hwnd);
    void refresh();
    HWND get_matching_hwnd(std::string title="", std::string path="");
    int activate(std::string title, std::string path);
    WndCoordinates get_coordinates(int window_index);
    int reposition(std::string title, std::string path, WndCoordinates coords,
                   int borderless, int alwaysontop);

    static BOOL CALLBACK EnumWindows_callback(HWND hwnd, LPARAM lp) {
        auto t = reinterpret_cast<OpenWindows*>(lp); // this
        auto title = t->get_title(hwnd);
        auto path = t->get_path(hwnd);
        bool valid = IsWindowVisible(hwnd) and not title.empty()
            and not t->should_exclude(title);
        if (valid) t->list.push_back({title, path, hwnd});
        return true;
    }
};
