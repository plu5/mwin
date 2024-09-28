#include "string_transformations.h"
#include <algorithm> // std::transform

// https://en.cppreference.com/w/cpp/string/byte/toupper "To use these
// functions safely with plain chars (or signed chars), the argument should
// first be converted to unsigned char"
char toupper_(unsigned char c) {
    return static_cast<char>(std::toupper(c));
}

std::string upper(const std::string& s) {
    std::string copy = s;
    std::transform(copy.begin(), copy.end(), copy.begin(), toupper_);
    return copy;
}
