#pragma once

struct Brush {
    HBRUSH hbr;
    Brush(int clr) {hbr = CreateSolidBrush(clr);}
    ~Brush() {DeleteObject(hbr);}
};
