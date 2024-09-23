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
(std::wstring title, std::wstring class_name, HINSTANCE hinst, int show) {
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
        MessageBoxW(nullptr, wstrMessage.c_str(), program_name, MB_ICONERROR);
        return nullptr;
    }

    HWND hwnd = CreateWindowW
        (instance->class_name.data(), instance->title.data(), WS_OVERLAPPEDWINDOW,
         CW_USEDEFAULT, 0, CW_USEDEFAULT, 0,
         nullptr, nullptr, hinst, instance.get());
    if (!hwnd) {
        std::wstring wstrMessage = L"create_window: CreateWindowW failed.\n\
Last error: " + std::to_wstring(GetLastError());
        MessageBoxW(nullptr, wstrMessage.c_str(), program_name, MB_ICONERROR);
        return nullptr;
    }
    instance->hwnd = hwnd;

    ShowWindow(hwnd, show);
    UpdateWindow(hwnd);
    return instance;
}

LRESULT CALLBACK Window::s_proc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
    Window* self;
    if (msg == WM_NCCREATE) {
        auto lpcs = reinterpret_cast<LPCREATESTRUCT>(lp);
        self = reinterpret_cast<Window*>(lpcs->lpCreateParams);
        self->hwnd = hwnd;
        SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LPARAM>(self));
    } else {
        self = reinterpret_cast<Window*>(GetWindowLongPtr
                                         (hwnd, GWLP_USERDATA));
    }
    if (self) {
        return self->proc(msg, wp, lp);
    } else {
        return DefWindowProc(hwnd, msg, wp, lp);
    }
}

LRESULT Window::proc(UINT msg, WPARAM wp, LPARAM lp) {
    LRESULT lres;
    switch (msg) {
    case WM_NCDESTROY:
        lres = DefWindowProc(hwnd, msg, wp, lp);
        SetWindowLongPtr(hwnd, GWLP_USERDATA, 0);
        return lres;
    }
    return DefWindowProc(hwnd, msg, wp, lp);
}
