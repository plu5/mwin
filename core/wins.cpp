#include "core/wins.h"
#include "utility/string_conversion.h" // wchar_to_string
#include "utility/win32_text.h" // get_window_text
#include <plog/Log.h>
#include <dwmapi.h> // DwmGetWindowAttribute
#pragma comment(lib, "dwmapi.lib")

bool OpenWindows::should_exclude(const std::string& str) {
    return exclusions.count(str) > 0;
}

std::string OpenWindows::get_title(HWND hwnd) {
    return get_window_text(hwnd);
}

std::string OpenWindows::get_path
(HWND hwnd, bool* access_denied /* =nullptr */) {
    DWORD id;
    GetWindowThreadProcessId(hwnd, &id);
    DWORD size = MAX_PATH;
    auto path = new wchar_t[size];
    HANDLE hproc = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, id);
    if (hproc) {
        QueryFullProcessImageNameW(hproc, 0, path, &size);
        CloseHandle(hproc);
        return wchar_to_string(path);
    } else {
        auto error = GetLastError();
        if (access_denied and error == 5) *access_denied = true;
        return {};
    }
}

void OpenWindows::refresh() {
    list.clear();
    EnumWindows(EnumWindows_callback, reinterpret_cast<LPARAM>(this));
}

HWND OpenWindows::get_matching_hwnd
(std::string title /* ="" */, std::string path /* ="" */) {
    std::map<int, HWND> found;
    for (const auto& w : list) {
        if (title.size()) {
            if (path.size()) {    // both title and path are provided
                if (title == w.title and path == w.path) return w.hwnd;
            } else {              // only title is provided
                if (title == w.title) return w.hwnd;
            }
        } else if (path.size()) { // only path is provided
            if (path == w.path) return w.hwnd;
        }
    }
    LOG_INFO << "Window with title '" << title << "' and path '" << path <<
        "' not found";
    return nullptr; // not found
}

void activate_hwnd(HWND hwnd) {
    SetForegroundWindow(hwnd);
}

int OpenWindows::activate(std::string title="", std::string path="") {
    refresh();
    if (auto hwnd = get_matching_hwnd(title, path)) {
        activate_hwnd(hwnd);
        return 1;
    } else {
        return -1;  // not found
    }
}

WndCoordinates get_coordinates_hwnd(HWND hwnd) {
    RECT rect;
    GetWindowRect(hwnd, &rect);
    return {rect.left, rect.top,
        rect.right - rect.left, rect.bottom - rect.top};
}

WndCoordinates OpenWindows::get_coordinates(int window_index) {
    return get_coordinates_hwnd(list.at(window_index).hwnd);
}

WndCoordinates calculate_shadow(HWND hwnd) {
    RECT exclude_shadow {};
    RECT include_shadow {};
    DwmGetWindowAttribute(hwnd, DWMWA_EXTENDED_FRAME_BOUNDS, &exclude_shadow,
                          sizeof(exclude_shadow));
    GetWindowRect(hwnd, &include_shadow);
    return {
        include_shadow.left - exclude_shadow.left,      // left
        include_shadow.top - exclude_shadow.top,        // top
        (include_shadow.right - include_shadow.left) -
        (exclude_shadow.right - exclude_shadow.left),   // width
        (include_shadow.bottom - include_shadow.top) -
        (exclude_shadow.bottom - exclude_shadow.top)    // height
    };
}

WndCoordinates add_shadow(HWND hwnd, WndCoordinates coords) {
    WndCoordinates shadow = calculate_shadow(hwnd);
    return {
        coords.x + shadow.x,
        coords.y + shadow.y,
        coords.w + shadow.w,
        coords.h + shadow.h
    };
}

void set_hwnd_pos(HWND hwnd, WndCoordinates coords, HWND z=HWND_TOP) {
    SetWindowPos(hwnd, z, coords.x, coords.y, coords.w, coords.h,
                 SWP_FRAMECHANGED);
}

namespace ws {  // window styles
    LONG visible = WS_VISIBLE;
    LONG borderless = WS_POPUP;
    LONG border = WS_OVERLAPPEDWINDOW;
}

void set_hwnd_style(HWND hwnd, LONG style) {
    SetWindowLongPtr(hwnd, GWL_STYLE, style);
}

bool style_flag_set(HWND hwnd, LONG flag) {
    LONG style = GetWindowLong(hwnd, GWL_STYLE);
    return (style & flag) == flag;
}

// for borderless and alwaysontop: 0 = apply, 1 = as is, 2 = unapply
int OpenWindows::reposition
(std::string title, std::string path, WndCoordinates coords, int borderless,
 int alwaysontop) {
    refresh();
    if (auto hwnd = get_matching_hwnd(title, path)) {
        LONG* style = nullptr;
        if (borderless == 0) style = &ws::borderless;
        else if (borderless == 2) style = &ws::border;
        if (style) set_hwnd_style(hwnd, ws::visible | *style);
        // NOTE(plu5): The following is a workaround for when the window was
        //  borderless and now we are restoring the borders, which for some
        //  reason causes it to set to the wrong size unless I set pos before
        //  calculating the shadow.
        if (borderless == 2) set_hwnd_pos(hwnd, coords);

        coords = add_shadow(hwnd, coords);
        set_hwnd_pos(hwnd, coords,
                     alwaysontop == 0 ? HWND_TOPMOST :
                     (alwaysontop == 2 ? HWND_NOTOPMOST : HWND_TOP));
        return 1;
    } else {
        return -1;  // not found
    }
}
