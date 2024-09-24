#include "save.h"
#include <fstream>
#include <filesystem>
#include "nlohmann/json.hpp"

using json = nlohmann::json;

auto save_rel_path = "mwin-save.json";
auto config_rel_path = "mwin-config.json";

bool load_file(std::filesystem::path path, std::ifstream& stream) {
    auto cpath = path.c_str();
    if (std::filesystem::exists(cpath)) {
        stream.open(cpath);
        return true;
    } else {
        return false;
    }
}

void save_json_file(std::filesystem::path path, json j) {
    std::ofstream o(path.c_str());
    o << std::setw(4) << j << std::endl;
}

void parse_failure
(const json::exception& e, const std::string& kind, const std::string& path) {
    // TODO(plu5): implement parse error handling + dialog
    // LOG_ERROR << e.what();
    // auto res = MainWindow->show_parse_failure_dialog(kind, path, e.what());
    // if (res != mrIgnore) {
    //     throw AbortRequested();
    // }
}

bool is_path_default_user_dir(std::filesystem::path path) {
    Config default_config;
    std::filesystem::path default_user_dir = default_config.user_dir.c_str();
    return path == default_user_dir;
}

bool load_config_(Config &config, std::filesystem::path path) {
    std::ifstream f;
    if (load_file(path, f)) {
        // TODO(plu5): add logging submodule + cout support
        // LOG_DEBUG << "Read config: " << path.c_str();
        try {
            config = json::parse(f);
        } catch (const json::exception& e) {
            parse_failure(e, "config", path.string());
            return false;
        }
        return true;
    } else {
//         LOG_DEBUG << "Config path " << path.c_str() << " not found.\n\
// This is normal if the config file has not been created yet.";
        return false;
    }
}

void load_config(Config &config) {
    std::filesystem::path user_dir = config.user_dir.c_str();
    std::filesystem::path path = user_dir / config_rel_path;
    if (!load_config_(config, path.c_str())) {
        // LOG_ERROR << "Failed to load config " << path.c_str();
        return;
    };
    user_dir = config.user_dir.c_str();
    if (not is_path_default_user_dir(user_dir)) {
        auto custom_path = user_dir / config_rel_path;
//         LOG_DEBUG << "Non-default user_dir: " << user_dir.c_str() << "\n\
// Attempting to load config from: " << custom_path.c_str();
        load_config_(config, custom_path);
    }
}

void save_config(Config &config) {
    std::filesystem::path user_dir = config.user_dir.c_str();
    std::filesystem::path path = user_dir / config_rel_path; 
    // LOG_DEBUG << "Saving config to " << path.c_str();
    save_json_file(path, config);
    if (not is_path_default_user_dir(user_dir)) {
        Config default_config;
        std::filesystem::path default_user_dir =
            default_config.user_dir.c_str();
        path = default_user_dir / config_rel_path;
//         LOG_DEBUG << "Non-default user_dir: " << user_dir.c_str() << "\n \
// Saving dconfig to: " << path.c_str();
        save_json_file(path, config);
    }
}
