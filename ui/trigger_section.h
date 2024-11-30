#pragma once

#include <Windows.h> // HWND, HINSTANCE
#include "ui/button.h" // Button
#include "ui/status.h" // Status

class TriggerSection {
public:
    int height = 35;
    Button trigger_btn;
    void initialise(HWND parent_hwnd_, HINSTANCE hinst_);
    void adjust_size();
    void set_font(HFONT font);
    void paint(HDC hdc);
    template<typename... T>
    void update_state(bool selector_state, T... other_selector_states) {
        if (selector_state or (... or other_selector_states))
            trigger_btn.enable();
        else trigger_btn.disable();
    }
protected:
    int y = 0;
    HWND parent_hwnd = NULL;
    HINSTANCE hinst = NULL;
    int btn_size = 25;
    int btn_wsize = 100;
    int marg = 5;
    Status status;
};
