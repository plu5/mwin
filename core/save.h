#pragma once

#include "core/config.h" // Config
#include "core/rules.h" // RulesModel
#include <filesystem> // std::filesystem::path

void load_config(Config &config);
void save_config(Config &config);
void load_rules(RulesModel &rules, std::filesystem::path user_dir);
void save_rules(RulesModel &rules, std::filesystem::path user_dir);
