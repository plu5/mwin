#include "utility/string_conversion.h"
#include <Windows.h>

// tfinniga https://stackoverflow.com/a/3999597/18396947
std::string wstring_to_string(const std::wstring &wstr) {
    if (wstr.empty()) return std::string();
    int size_needed = WideCharToMultiByte
        (CP_UTF8, 0, &wstr[0], (int)wstr.size(), NULL, 0, NULL, NULL);
    std::string res(size_needed, 0);
    WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), &res[0],
                        size_needed, NULL, NULL);
    return res;
}
