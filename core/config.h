#pragma once

#include "core/coords.h"
#include "nlohmann/json.hpp"

struct Config {
    std::string user_dir;
    WndCoordinates window_geometry;
    bool save_geom_on_quit = true;
    Config();
    void load();
    void save();
    // noncopyable (don't want several configs with different states)
    Config(const Config&) = delete;
    Config& operator=(const Config&) = delete;
    Config(Config&&) = default;
    Config& operator=(Config&&) = default;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT\
(Config, user_dir, window_geometry, save_geom_on_quit)
