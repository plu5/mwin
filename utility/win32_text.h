#pragma once

#include <Windows.h>
#include <string> // std::string, std::wstring
#include "utility/string_conversion.h" // wstring_to_string

inline std::string get_window_text(HWND hwnd) {
    auto length_with_terminator = GetWindowTextLength(hwnd) + 1;
    std::wstring text(length_with_terminator, '\0');
    auto length_copied = GetWindowText(hwnd, text.data(), length_with_terminator);
    text.resize(length_copied); // get rid of excess \0
    auto res = wstring_to_string(text);
    return res;
}
