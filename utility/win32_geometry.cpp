#include "utility/win32_geometry.h"

RECT get_rect(HWND hwnd, bool client) {
    RECT rect {};
    if (client) GetClientRect(hwnd, &rect);
    else GetWindowRect(hwnd, &rect);
    return rect;
}

// thinlizzy https://stackoverflow.com/a/20535798/18396947
RECT get_relative_rect(HWND hwnd, HWND parent) {
    RECT rect {};
    GetClientRect(hwnd, &rect);
    MapWindowPoints(hwnd, parent, reinterpret_cast<POINT*>(&rect), 2);
    return rect;
}

Size get_size(HWND hwnd, bool client) {
    auto rect = get_rect(hwnd, client);
    return {rect.right - rect.left, rect.bottom - rect.top, rect};
}
