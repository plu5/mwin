#include <Windows.h>
#include <string> // std::wstring
#include "ui/base.h" // Window, create_window
#include "utility/win32_painting.h" // Font, paint_text, paint_rect

class IdentifyIndicator : public Window {
    using super = Window;
public:
    HWND hwnd = NULL;
    int x = 0, y = 0, w = 250, h = 100;
    IdentifyIndicator
    (std::wstring title, std::wstring class_name, HINSTANCE hinst)
        : Window(title, class_name, hinst) {initialise();};
    void show(int x_, int y_, const std::wstring& text_);
    void hide();
protected:
    int timer_id = 1;
    int timer_interval = 1000; // msec
    int initial_seconds_left = 2, seconds_left = 2;
    std::wstring text = L"";
    std::wstring closing_label_template = L"closing in {} seconds";
    std::wstring closing_label = L"";
    Font big_font;
    std::wstring big_font_face = L"MS Shell Dlg 2";
    int big_font_size = 20;
    int top_bar_height = 20;
    COLORREF top_bar_colour = RGB(0, 220, 220);
    int closing_label_bottom_margin = 5;
    void initialise();
    void update_closing_label();
    LRESULT proc(UINT msg, WPARAM wp, LPARAM lp) override;
    void paint();
};
