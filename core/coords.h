#pragma once

#include "nlohmann/json.hpp"

struct WndCoordinates {
    int x = 0; 
    int y = 0;
    int w = 0;
    int h = 0;
    WndCoordinates() {};
    WndCoordinates(int x, int y, int w, int h) : x(x), y(y), w(w), h(h) {};
    inline void unpack(int& x_, int& y_, int& w_, int& h_) const {
        x_ = x;
        y_ = y;
        w_ = w;
        h_ = h;
    }
    void by_monitor(size_t i);
    int& operator[](size_t i);
    const int& operator[](size_t i) const;
    bool operator==(const WndCoordinates& rhs) const;
    bool operator!=(const WndCoordinates& rhs) const;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(WndCoordinates, x, y, w, h)
