#pragma once

#include "constants.h" // ID::wname
#include "resource.h" // IDI_MWIN, IDC_MWIN
#include "core/coords.h" // WndCoordinates
#include "utility/win32_painting.h" // CompatDc, CompatBitmap
#include <Windows.h>

class Window {
public:
    HINSTANCE hinst;
    std::wstring title;
    std::wstring class_name;
    HWND hwnd = NULL;
    static LRESULT CALLBACK s_proc
    (HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);
    ~Window() {DestroyWindow(hwnd);}
protected:
    Window
    (std::wstring title, std::wstring class_name, HINSTANCE hinst) :
        title(title), class_name(class_name), hinst(hinst) {};
    virtual LRESULT proc(UINT msg, WPARAM wp, LPARAM lp);
    // Painting & double-buffering
    HDC hdc1 = NULL;
    CompatDc dc2;
    CompatBitmap bmp;
    void setup_paint_buffers();
    virtual void paint_bg(HDC hdc);
    virtual void paint();
};

template<typename T>
HWND create_window
(T& instance, HINSTANCE hinst, WndCoordinates* geometry=nullptr,
 int flags=WS_OVERLAPPEDWINDOW, HWND parent=nullptr, bool menu=true) {
    WNDCLASSEXW wcex {};
    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = T::s_proc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hinst;
    wcex.hIcon          = parent ? NULL :
        LoadIcon(hinst, MAKEINTRESOURCE(IDI_MWIN));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = NULL;
    wcex.lpszMenuName   = (parent or not menu) ? NULL : MAKEINTRESOURCEW(IDC_MWIN);
    wcex.lpszClassName  = instance.class_name.data();
    wcex.hIconSm        = parent ? NULL :
        LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_MWIN));

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
        (instance.class_name.data(), instance.title.data(),
         flags,
         x, y, w, h,
         parent, nullptr, hinst, &instance);
    if (!hwnd) {
        std::wstring wstrMessage = L"create_window: CreateWindowW failed.\n\
Last error: " + std::to_wstring(GetLastError());
        MessageBoxW(nullptr, wstrMessage.c_str(), ID::wname, MB_ICONERROR);
        return nullptr;
    }
    instance.hwnd = hwnd;
    return hwnd;
}

template<typename T>
std::unique_ptr<T> create_window
(std::wstring title, std::wstring class_name, HINSTANCE hinst,
 WndCoordinates* geometry=nullptr, int flags=WS_OVERLAPPEDWINDOW,
 HWND parent=nullptr) {
    auto instance = std::unique_ptr<T>
        (new T(title, class_name, hinst));
    auto hwnd = create_window
        (*instance.get(), hinst, geometry, flags, parent);
    if (!hwnd) return nullptr;
    return instance;
}
