#include "mwin.h"
#include "ui/base.h"
#include "ui/main_window.h"
#include "utility/safe_resources.h"

const WCHAR program_name[] = L"mwin";

int APIENTRY wWinMain
(_In_ HINSTANCE hinst, _In_opt_ HINSTANCE, _In_ LPWSTR, _In_ int show) {
    auto window = create_window<MainWindow>
        (load_wstr_resource(hinst, IDS_APP_TITLE),
         load_wstr_resource(hinst, IDC_MWIN),
         hinst, show);

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

    return (int)msg.wParam;
}
