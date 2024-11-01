#pragma once

#include <string>
#include "core/coords.h"

enum class RuleFieldType {none, id, enabled, name, commentary, wnd_title,
    wnd_exe, coords, monitor, borderless};

struct RuleFieldData {
    std::string str;
    bool boolean;
    WndCoordinates coords;
    int num;
};

struct RuleFieldChange {
    RuleFieldType field = RuleFieldType::none;
    RuleFieldData data;
};

struct Rule {
    std::string id;
    bool enabled = true;
    std::string name = "";
    std::string commentary = "";
    std::string wnd_title = "";
    std::string wnd_exe = "";
    WndCoordinates coords;
    int monitor = 1;
    int borderless = 1;
    RuleFieldData get(RuleFieldType field) const;
    void set(const RuleFieldChange& change);
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT
(Rule, id, enabled, name, commentary, wnd_title, wnd_exe, coords, monitor,
 borderless)

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
