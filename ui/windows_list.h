#pragma once

#include <Windows.h> // HWND
#include <optional> // std::optional
#include "core/coords.h" // WndCoordinates
#include "core/wins.h" // OpenWindows

enum class WindowFieldType {none, hwnd, title, path, geometry};

struct WindowFieldData {
    std::optional<std::string> str;
    std::optional<WndCoordinates> coords;
};

class WindowsList {
public:
    HWND hwnd = 0;
    OpenWindows wins;
    void initialise
    (HWND parent_hwnd_, HINSTANCE hinst_, const WndCoordinates& geom);
    void resize(int w_, int h_);
    void refresh();
    int selected();
    WindowFieldData get(WindowFieldType type, size_t i);
    static LRESULT CALLBACK s_proc
    (HWND hwnd, UINT msg, WPARAM wp, LPARAM lp, UINT_PTR uid, DWORD_PTR);
protected:
    int x = 0, y = 0, w = 0, h = 0;
    HWND parent_hwnd = 0;
    HINSTANCE hinst = 0;
    LRESULT proc(UINT msg, WPARAM wp, LPARAM lp);
};
