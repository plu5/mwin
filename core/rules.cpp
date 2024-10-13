#include "core/rules.h"
#include "utility/uid.h"
#include "utility/string_concat.h"

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
    if (r.by_monitor) r.coords.by_monitor(r.monitor - 1);
    rules.push_back(r);
    return rules[rules.size() - 1];
}

Rule& RulesModel::duplicate_rule(int index) {
    Rule copied_rule = rules[index];
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
