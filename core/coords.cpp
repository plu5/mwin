#include "core/coords.h"
#include <vector>
#include <tuple> // std::tie
#include <plog/Log.h>
#include "utility/monitors.h"

void WndCoordinates::by_monitor(size_t i) {
    Monitors monitors;
    if (i > monitors.rects.size()) {
        LOG_ERROR << "Monitor " << i << " not found";
        return;
    }
    auto& m = monitors.rects.at(i);
    x = m.left;
    y = m.top;
    w = m.right - m.left;
    h = m.bottom - m.top;
}

int& WndCoordinates::operator[](size_t i) {
    // Avoid duplication of the other operator[] overload
    // "It’s okay to remove the const on a const reference to a non-const
    // object."
    // https://www.learncpp.com/cpp-tutorial/overloading-the-subscript-operator
    return const_cast<int&>(const_cast<const WndCoordinates&>(*this)[i]); 
}

const int& WndCoordinates::operator[](size_t i) const {
    switch(i) {
    case 0: return x;
    case 1: return y;
    case 2: return w;
    case 3: return h;
    default: throw std::invalid_argument("invalid index");
    }
}

bool WndCoordinates::operator==(const WndCoordinates& rhs) const {
    return std::tie(x, y, w, h) == std::tie(rhs.x, rhs.y, rhs.w, rhs.h);
}

bool WndCoordinates::operator!=(const WndCoordinates& rhs) const {
    return !operator==(rhs);
}
