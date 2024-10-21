#pragma once

#include <string> // std::string, std::wstring
#include <format> // std::vformat, std::make_format_args, ...

template<typename... Ts>
std::string format(const std::string& s, const Ts& ... ts) {
    return std::vformat(s, std::make_format_args(ts...));
}

template<typename... Ts>
std::wstring wformat(const std::wstring& s, const Ts& ... ts) {
    return std::vformat(s, std::make_wformat_args(ts...));
}
