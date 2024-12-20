#include "Windows.h"
#include "res/resource.h"
#include "ui/base.h"
#include "ui/main_window.h"
#include "utility/safe_resources.h"
#include "utility/console_output_support.h"
#include "core/cli.h"
#include "plog/Log.h"

int APIENTRY wWinMain
(_In_ HINSTANCE hinst, _In_opt_ HINSTANCE, _In_ LPWSTR, _In_ int) {
    auto con_support = ConsoleOutputSupport();
    cli::setup(__argc, __wargv, con_support);

    auto window = create_window<MainWindow>
        (load_wstr_resource(hinst, IDS_APP_TITLE),
         load_wstr_resource(hinst, IDC_MWIN),
         hinst, nullptr,
         WS_OVERLAPPEDWINDOW & ~(WS_MINIMIZEBOX | WS_MAXIMIZEBOX) |
         WS_CLIPCHILDREN | WS_CLIPSIBLINGS);

    // "If the function terminates before entering the message loop, it should
    // return zero."
    if (!window) return 0;

    HACCEL accel_table = LoadAccelerators(hinst, MAKEINTRESOURCE(IDC_MWIN));
    MSG msg;

    while (GetMessage(&msg, nullptr, 0, 0)) {
        if (!TranslateAccelerator(msg.hwnd, accel_table, &msg)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    LOG_INFO << ID::name << " quit";
    return (int)msg.wParam;
}
