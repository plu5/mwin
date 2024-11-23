#pragma once

#include <Windows.h> // HWND, HINSTANCE
#include "ui/button.h" // Button

class TriggerSection {
public:
    HWND hwnd = NULL;
    int height = 35;
    Button trigger_btn;
    void initialise(HWND parent_hwnd_, HINSTANCE hinst_);
    void adjust_size();
    void paint(HDC hdc);
protected:
    int y = 0;
    HWND parent_hwnd = NULL;
    HINSTANCE hinst = NULL;
    int btn_size = 25;
    int btn_wsize = 100;
    int marg = 5;
};
