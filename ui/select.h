#pragma once

#include <Windows.h>
#include <string> // std::string

class Select {
public:
    HWND hwnd = NULL;
    void initialise
    (HWND parent_, HINSTANCE hinst_, int x_, int y_, int w_, int h_);
    void add_option(std::string s);
    void select(int i, bool enable=true);
    int selected() const;
    void resize_width(int w_);
    void clear_and_disable();
protected:
    int x = 0, y = 0, w = 0, h = 0;
    HWND parent = NULL;
    HINSTANCE hinst = NULL;
};
