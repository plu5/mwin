#include "core/config.h"
#include "core/save.h"
#include "utility/exe_path.h"

Config::Config() {
    user_dir = get_current_executable_root();
}

void Config::load() {
    load_config(*this);
}

void Config::save() {
    save_config(*this);
}
