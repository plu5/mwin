#pragma once

#include <string>

// Caleth https://stackoverflow.com/a/72963617/18396947
namespace string_builder {
    std::size_t length(char) {
        return 1;
    }
    
    void append(std::string& dest, char str) {
        dest.push_back(str);
    }
    
    std::size_t length(const char* str) {
        return std::strlen(str);
    }
    
    void append(std::string& dest, const char* str) {
        dest.append(str);
    }
    
    template<std::size_t N>
    std::size_t length(const char (&str)[N]) {
        return N;
    }
    
    template<std::size_t N>
    void append(std::string& dest, const char (&str)[N]) {
        dest.append(str, N);
    }
    
    std::size_t length(const std::string& str) {
        return str.size();
    }
    
    void append(std::string& dest, const std::string& str) {
        dest.append(str);
    }
}

template<typename... Ts>
std::string concat(const Ts &... ts) {
    std::string result;
    result.reserve((string_builder::length(ts) + ...));
    (string_builder::append(result, ts), ...);
    return result;
}
