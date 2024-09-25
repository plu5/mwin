#include "console_output_support.h"
#include "Windows.h"
#include <cstdio>

ConsoleOutputSupport::ConsoleOutputSupport() {
    attach();
}

ConsoleOutputSupport::~ConsoleOutputSupport() {
    detach();
}

// VRHans https://forum.qt.io/post/314960 -- modified
bool ConsoleOutputSupport::attach() {
    if (attached) return false;
    // If we were launched from the command line, we should usually be able to
    // attach to the console
    if (AttachConsole(ATTACH_PARENT_PROCESS)) {
        attached = true;
        // Re-open the streams to the console
        FILE* stream;
        freopen_s(&stream, "CONOUT$", "w", stdout);
        freopen_s(&stream, "CONOUT$", "w", stderr);
        freopen_s(&stream, "CONIN$", "r", stdin);
        return true;
    }
    return false;
}

bool ConsoleOutputSupport::detach() {
    if (!attached) return true;
    // Send an enter key as the console being freed is not enough to release it
    HANDLE hConsoleInputHandle = GetStdHandle(STD_INPUT_HANDLE);
    INPUT_RECORD aInputRecords[1] {};
    DWORD nEventsWritten = 0;
    if (hConsoleInputHandle == NULL) return false;
    aInputRecords[0].EventType = KEY_EVENT;
    aInputRecords[0].Event.KeyEvent.bKeyDown = TRUE;
    aInputRecords[0].Event.KeyEvent.uChar.UnicodeChar = VK_RETURN;
    aInputRecords[0].Event.KeyEvent.wVirtualKeyCode = VK_RETURN;
    aInputRecords[0].Event.KeyEvent.wVirtualScanCode =
        (WORD)MapVirtualKey(VK_RETURN, MAPVK_VK_TO_VSC);
    aInputRecords[0].Event.KeyEvent.wRepeatCount = 1;
    aInputRecords[0].Event.KeyEvent.dwControlKeyState = 0;
    WriteConsoleInput(hConsoleInputHandle, aInputRecords, 1, &nEventsWritten);
    return FreeConsole() ? true : false;
}
