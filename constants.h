#pragma once

struct ID {
    constexpr static const char* const name = "mwin";
    constexpr static const wchar_t* const wname = L"mwin";
    constexpr static const char* const version = "1.0.0";
};

struct Theme {
    constexpr static const int bg = 0x00101010;
    constexpr static const int fg = 0x00DCDCDC;
    constexpr static const int details_bg = 0x00252526;
    constexpr static const int edits_bg = 0x001E1E1E;
    constexpr static const int edits_line = 0x002B2B2B;
    constexpr static const int sep_bg = 0x00171717;
    constexpr static const int sep_fg = 0x00404040;
    constexpr static const int border = 0x00646464;
};

struct UM { // custom message ids
    constexpr static const int post_init = 32788;
    constexpr static const int grabbed = 32789;
    constexpr static const int rule_deselected = 32790;
};
