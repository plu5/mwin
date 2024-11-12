#include "ui/windows_list.h"
#include <commctrl.h> // WC_LISTBOX
#include <windowsx.h> // ListBox macros
#include "utility/win32_casts.h" // hmenu_cast
#include "utility/win32_geometry.h" // get_size, get_rect
#include "utility/string_conversion.h" // string_to_wstring

HWND create_listbox
(int x, int y, int w, int h, int id, HWND parent, HINSTANCE hinst,
 DWORD extra_flags=LBS_NOINTEGRALHEIGHT | WS_VSCROLL | LBS_NOTIFY) {
    return CreateWindow
        (WC_LISTBOX, L"", WS_CHILD | WS_VISIBLE | extra_flags,
         x, y, w, h, parent, hmenu_cast(id), hinst, NULL);
}

void WindowsList::initialise
(HWND parent_hwnd_, HINSTANCE hinst_, const WndCoordinates& geom) {
    parent_hwnd = parent_hwnd_;
    hinst = hinst_;
    geom.unpack(x, y, w, h);

    hwnd = create_listbox(x, y, w, h, -1, parent_hwnd, hinst);
    SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LPARAM>(this));
    SetWindowSubclass(hwnd, s_proc, static_cast<UINT_PTR>(-1), 0);
}

void WindowsList::resize(int w_, int h_) {
    w = w_;
    h = h_;
    SetWindowPos(hwnd, NULL, 0, 0, w, h, SWP_NOMOVE);
}

void WindowsList::refresh() {
    wins.refresh();
    ListBox_ResetContent(hwnd);
    for (const auto& win : wins.list) {
        auto s = string_to_wstring(win.title);
        ListBox_AddString(hwnd, s.data());
    }
}

int WindowsList::selected() {
    return ListBox_GetCurSel(hwnd);
}

LRESULT CALLBACK WindowsList::s_proc
(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp, UINT_PTR uid, DWORD_PTR) {
    if (msg == WM_NCDESTROY) {
        RemoveWindowSubclass(hwnd, s_proc, uid);
        return DefSubclassProc(hwnd, msg, wp, lp);
    }
    auto self = reinterpret_cast<WindowsList*>
        (GetWindowLongPtr(hwnd, GWLP_USERDATA));
    if (self) {
        return self->proc(msg, wp, lp);
    } else {
        return DefSubclassProc(hwnd, msg, wp, lp);
    }
}

LRESULT WindowsList::proc
(UINT msg, WPARAM wp, LPARAM lp) {
    switch (msg) {
    case WM_KEYDOWN:
        if (wp == VK_RETURN) PostMessage(parent_hwnd, msg, wp, lp);
        break;
    }
    return DefSubclassProc(hwnd, msg, wp, lp);
}

WindowFieldData WindowsList::get(WindowFieldType type, size_t i) {
    if (type == WindowFieldType::title) {
        return {.str = wins.list.at(i).title};
    } else if (type == WindowFieldType::path) {
        return {.str = wins.list.at(i).path};
    } else if (type == WindowFieldType::geometry) {
        return {.coords = wins.get_coordinates(static_cast<int>(i))};
    }
    return {};
}
