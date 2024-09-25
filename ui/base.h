#pragma once

#include "constants.h"
#include "resource.h"
#include "core/coords.h"
#include <Windows.h>

class Window {
public:
    HINSTANCE hinst;
    int show;
    std::wstring title;
    std::wstring class_name;
    HWND hwnd = 0;
    static LRESULT CALLBACK s_proc
    (HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);
protected:
    Window
    (std::wstring title, std::wstring class_name, HINSTANCE hinst, int show)
        : title(title), class_name(class_name), hinst(hinst), show(show) {};
    virtual LRESULT proc(UINT msg, WPARAM wp, LPARAM lp);
};

template<typename T>
std::unique_ptr<T> create_window
(std::wstring title, std::wstring class_name, HINSTANCE hinst, int show,
 WndCoordinates* geometry=nullptr) {
    auto instance = std::unique_ptr<T>
        (new T(title, class_name, hinst, show));

    WNDCLASSEXW wcex {};
    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = T::s_proc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hinst;
    wcex.hIcon          = LoadIcon(hinst, MAKEINTRESOURCE(IDI_MWIN));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_MWIN);
    wcex.lpszClassName  = instance->class_name.data();
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_MWIN));

    if (RegisterClassExW(&wcex) == 0) {
        std::wstring wstrMessage = L"create_window: RegisterClassExW failed.\n\
Last error: " + std::to_wstring(GetLastError());
        MessageBoxW(nullptr, wstrMessage.c_str(), ID::wname, MB_ICONERROR);
        return nullptr;
    }

    int x = CW_USEDEFAULT, y = CW_USEDEFAULT,
        w = CW_USEDEFAULT, h = CW_USEDEFAULT;
    if (geometry) geometry->unpack(x, y, w, h);

    HWND hwnd = CreateWindowW
        (instance->class_name.data(), instance->title.data(),
         WS_OVERLAPPEDWINDOW,
         x, y, w, h,
         nullptr, nullptr, hinst, instance.get());
    if (!hwnd) {
        std::wstring wstrMessage = L"create_window: CreateWindowW failed.\n\
Last error: " + std::to_wstring(GetLastError());
        MessageBoxW(nullptr, wstrMessage.c_str(), ID::wname, MB_ICONERROR);
        return nullptr;
    }
    instance->hwnd = hwnd;

    return instance;
}
