#include "core/rules.h"
#include "utility/uid.h"
#include "utility/string_concat.h"

RuleFieldData Rule::get(RuleFieldType field) const {
    RuleFieldData data {};
    switch (field) {
    case RuleFieldType::id:
        data.str = id;
        break;
    case RuleFieldType::enabled:
        data.boolean = enabled;
        break;
    case RuleFieldType::name:
        data.str = name;
        break;
    case RuleFieldType::commentary:
        data.str = commentary;
        break;
    case RuleFieldType::wnd_title:
        data.str = wnd_title;
        break;
    case RuleFieldType::wnd_exe:
        data.str = wnd_exe;
        break;
    case RuleFieldType::coords:
        data.coords = coords;
        break;
    case RuleFieldType::monitor:
        data.num = monitor;
        break;
    case RuleFieldType::borderless:
        data.num = borderless;
        break;
    case RuleFieldType::alwaysontop:
        data.num = alwaysontop;
        break;
    }
    return data;
}

void Rule::set(const RuleFieldChange& change) {
    switch (change.field) {
    case RuleFieldType::id:
        id = change.data.str;
        break;
    case RuleFieldType::enabled:
        enabled = change.data.boolean;
        break;
    case RuleFieldType::name:
        name = change.data.str;
        break;
    case RuleFieldType::commentary:
        commentary = change.data.str;
        break;
    case RuleFieldType::wnd_title:
        wnd_title = change.data.str;
        break;
    case RuleFieldType::wnd_exe:
        wnd_exe = change.data.str;
        break;
    case RuleFieldType::coords:
        coords = change.data.coords;
        break;
    case RuleFieldType::monitor:
        monitor = change.data.num;
        break;
    case RuleFieldType::borderless:
        borderless = change.data.num;
        break;
    case RuleFieldType::alwaysontop:
        alwaysontop = change.data.num;
        break;
    }
}

// Similar to Notepad++ nextUntitledNewNumber
std::string RulesModel::next_untitled_name(std::string base, std::string sep) {
    std::string name = base;
    int number = 0;
    bool number_available = true;
    bool found = false;
    do {
        for (const auto& rule : rules) {
            number_available = true;
            found = false;
            if (rule.name == name) {
                number_available = false;
                found = true;
                break;
            }
        }
        if (!number_available) {
            number++;
            name = concat(base, sep, std::to_string(number));
        }
        if (!found) break;
    } while (!number_available);

    return name;
}

Rule& RulesModel::create_rule() {
    auto r = Rule({.id = uid(), .name = next_untitled_name()});
    if (r.monitor > 0) r.coords.by_monitor(r.monitor - 1);
    rules.push_back(r);
    return rules[rules.size() - 1];
}

Rule& RulesModel::duplicate_rule(int index) {
    Rule copied_rule = rules[index];
    copied_rule.id = uid();
    copied_rule.name = next_untitled_name(copied_rule.name);
    rules.push_back(copied_rule);
    return rules[rules.size() - 1];
}

void RulesModel::delete_rule(int index) {
    rules.erase(rules.begin() + index);
}

size_t RulesModel::size() {
    return rules.size();
}

Rule& RulesModel::operator[](int index) {
    return rules.at(index);
}

const Rule& RulesModel::operator[](int index) const {
    return rules.at(index);
}

const Rule& RulesModel::by_id(const std::string& id) const {
    for (const auto& r : rules) if (r.id == id) return r;
    throw std::invalid_argument("invalid rule id");
}

size_t RulesModel::index(const std::string& id) const {
    auto it = std::find_if(rules.begin(), rules.end(),
                           [&id](const Rule& r){return id == r.id;});
    return it - rules.begin();
}
