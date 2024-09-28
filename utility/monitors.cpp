#include "utility/monitors.h"

Monitors::Monitors() {
    EnumDisplayMonitors(NULL, NULL, enum_callback, (LPARAM)this);
}

BOOL CALLBACK Monitors::enum_callback
(HMONITOR, HDC, LPRECT rect, LPARAM lp) {
    auto pthis = reinterpret_cast<Monitors*>(lp);
    pthis->rects.push_back(*rect);
    return TRUE;
}
