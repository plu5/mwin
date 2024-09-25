#include "string_transformations.h"
#include <algorithm>
#include <cstring>

std::string upper(const std::string& s) {
    std::string copy = s;
    std::transform(copy.begin(), copy.end(), copy.begin(), std::toupper);
    return copy;
}
