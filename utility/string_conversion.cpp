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

std::string wchar_to_string(const wchar_t *wchr) {
    if (not wchr) return std::string();
    int size_needed = WideCharToMultiByte
        (CP_UTF8, 0, wchr, -1, NULL, 0, NULL, NULL) - 1;
    std::string res(size_needed, 0);
    WideCharToMultiByte(CP_UTF8, 0, wchr, -1, &res[0],
                        size_needed, NULL, NULL);
    return res;
}

std::wstring string_to_wstring(const std::string &str) {
    if(str.empty()) return std::wstring();
    int size_needed = MultiByteToWideChar
        (CP_UTF8, 0, &str[0], (int)str.size(), NULL, 0);
    std::wstring res(size_needed, 0);
    MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), &res[0], size_needed);
    return res;
}
