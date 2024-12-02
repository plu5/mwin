#include <ui/tooltip.h>

void Tooltip::initialise
(HWND parent, const std::wstring& text_, const RECT& rect,
 int max_width /* =-1 */) {
    delete_if_initialised();
    text = text_;
    hwnd = CreateWindowEx
        (WS_EX_TOPMOST, TOOLTIPS_CLASS, NULL,
         WS_POPUP | TTS_NOPREFIX | TTS_ALWAYSTIP,
         0, 0, 0, 0, parent, NULL, 0, NULL);
    info.cbSize = sizeof(TTTOOLINFO);
    info.uFlags = TTF_SUBCLASS;
    info.hwnd = parent;
    info.lpszText = text.data();
    info.rect = rect;
    SendMessage(hwnd, TTM_ADDTOOL, 0, reinterpret_cast<LPARAM>(&info));
    if (max_width != -1) set_max_width(max_width);
    initialised = true;
}

void Tooltip::delete_if_initialised() {
    if (initialised) {
        SendMessage(hwnd, TTM_DELTOOL, 0, reinterpret_cast<LPARAM>(&info));
        DestroyWindow(hwnd);
        initialised = false;
    }
}

Tooltip::~Tooltip() {
    delete_if_initialised();
}

void Tooltip::set_max_width(int max_width) {
    if (hwnd) SendMessage(hwnd, TTM_SETMAXTIPWIDTH, 0,
                          static_cast<LPARAM>(max_width));
}

void Tooltip::change_rect(const RECT& rect) {
    if (hwnd) {
        info.rect = rect;
        SendMessage(hwnd, TTM_NEWTOOLRECT, 0, reinterpret_cast<LPARAM>(&info));
    }
}
