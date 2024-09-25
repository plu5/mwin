#pragma once

#include <vector>
#include <string>

template<typename V>
std::string vector_join
(size_t size, V v, size_t start_index=0, std::string delim=" ") {
    std::string res = "";
    for (auto i = start_index; i < size; ++i) {
        res += v[i];
        if (i < size - 1) res += delim;
    }
    return res;
}

template<typename V>
std::wstring vector_join_w
(size_t size, V v, size_t start_index=0, std::wstring delim=L" ") {
    std::wstring res = L"";
    for (auto i = start_index; i < size; ++i) {
        res += v[i];
        if (i < size - 1) res += delim;
    }
    return res;
}

std::string vector_join(std::vector<std::string> v, std::string delim=" ") {
    return vector_join(v.size(), v, 0, delim);
}

std::wstring vector_join_w
(std::vector<std::wstring> v, std::wstring delim=L" ") {
    return vector_join_w(v.size(), v, 0, delim);
}
