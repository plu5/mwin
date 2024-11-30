#include "ui/trigger_section.h"
#include "utility/win32_geometry.h" // get_size, get_rect
#include "utility/win32_painting.h" // paint_rect
#include "constants.h" // Theme 

void TriggerSection::initialise(HWND parent_hwnd_, HINSTANCE hinst_) {
    parent_hwnd = parent_hwnd_;
    hinst = hinst_;
    auto size = get_size(parent_hwnd);
    trigger_btn.initialise
        (parent_hwnd, hinst,
         size.rect.right - btn_wsize - marg,
         size.rect.bottom - height/2 - btn_size/2,
         btn_wsize, btn_size, L"Trigger Rule");
    status.initialise(parent_hwnd, hinst);
}

void TriggerSection::adjust_size() {
    auto size = get_size(parent_hwnd);
    trigger_btn.reposition(size.rect.right - btn_wsize - marg,
                           size.rect.bottom - height/2 - btn_size/2);
    status.adjust_size({size.rect.left + marg, size.rect.bottom - height,
                       size.rect.right - btn_wsize - 2*marg, size.rect.bottom});
}

void TriggerSection::set_font(HFONT font) {
    status.set_font(font);
}

void TriggerSection::paint(HDC hdc) {
    // Trigger section border
    auto rc = get_rect(parent_hwnd);
    rc.top = rc.bottom - height;
    rc.bottom = rc.top + 1;
    paint_rect(hdc, Theme::border, &rc);
    status.paint(hdc);
}
