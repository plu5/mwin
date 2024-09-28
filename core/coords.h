#pragma once

#include "nlohmann/json.hpp"

struct WndCoordinates {
    int x = 0; 
    int y = 0;
    int w = 0;
    int h = 0;
    WndCoordinates() {};
    inline void unpack(int& x_, int& y_, int& w_, int& h_) const {
        x_ = x;
        y_ = y;
        w_ = w;
        h_ = h;
    }
    void by_monitor(size_t i);
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(WndCoordinates, x, y, w, h)
