#pragma once

#include <string>
#include "core/coords.h"

struct Rule {
    std::string id;
    bool enabled = true;
    std::string name = "";
    std::string commentary = "";
    std::string wnd_title = "";
    std::string wnd_exe = "";
    WndCoordinates coords;
    bool by_monitor = true;
    size_t monitor = 1;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT
(Rule, id, enabled, name, commentary, wnd_title, wnd_exe, coords, by_monitor,
 monitor)

class RulesModel {
 public:
    std::vector<Rule> rules;
    Rule& create_rule();
    Rule& duplicate_rule(int index);
    void delete_rule(int index);
    size_t size();
    Rule& operator[](int index);
    const Rule& operator[](int index) const;
    const Rule& by_id(const std::string& id) const;
    size_t index(const std::string& id) const;
 private:
    std::string next_untitled_name
    (std::string base="Untitled", std::string sep="-");
};
