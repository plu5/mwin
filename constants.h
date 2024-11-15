#pragma once

struct ID {
    constexpr static const char* const name = "mwin";
    constexpr static const wchar_t* const wname = L"mwin";
    constexpr static const char* const version = "1.0.0";
};

struct Theme {
    constexpr static const int bg = 0x00252526;
    constexpr static const int fg = 0x00DCDCDC;
    constexpr static const int edits_bg = 0x001E1E1E;
    constexpr static const int edits_border = 0x00646464;
};

struct UM { // custom message ids
    constexpr static const int post_init = 32788;
    constexpr static const int grabbed = 32789;
};
