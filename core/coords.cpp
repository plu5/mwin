#include "core/coords.h"
#include <vector>
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
