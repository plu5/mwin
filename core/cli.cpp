#include "core/cli.h"
#include <map>
#include <regex>
#include "plog/Log.h"
#include "plog/Initializers/ConsoleInitializer.h"
#include "plog/Formatters/TxtFormatter.h"
#include "constants.h"
#include "utility/word_wrap.h"
#include "utility/vector_join.h"
#include "utility/string_concat.h"
#include "utility/string_conversion.h"
#include "utility/string_transformations.h"

std::string program_str = concat(ID::name, " ", ID::version);
std::string usage_str = concat("usage: ", ID::name, " [-h] [-l LOGLEVEL]");
std::string err_prefix = concat(ID::name, ": error: ");
std::map<std::string, cli::Option> options = {
    {"help", {{"-h", "--help"}, "Show this help message and exit"}},
    {"loglevel",
     {{"-l", "--loglevel"},
      "Set logging output to one of: ", true,
      {"FATAL", "ERROR", "WARN", "INFO", "DEBUG", "VERB", "NONE"}}}
};

bool arg_equal
(const std::string& arg, std::vector<std::string> option_aliases) {
    for (const auto& alias : option_aliases)
        if (arg == alias) return true;
    return false;
}

std::string generate_help_str() {
    std::string help_str = usage_str + "\n\n" + program_str +
        "\n\noptional arguments:\n";
    for (const auto& [name, option] : options) {
        std::string line = "  ";
        for (size_t i = 0; i < option.aliases.size(); ++i) {
            line += option.aliases[i];
            if (option.takes_value) line += " " + upper(name);
            if (i < option.aliases.size() - 1) {
                line += ", ";
            } else {
                if (line.length() > 15) line += "\n\t";
                line += "\t";
                std::string desc = option.desc;
                if (option.possible_values.size() > 0)
                    desc += vector_join(option.possible_values, ", ");
                std::string wrapped_desc = wrap(desc, 56);
                line += std::regex_replace
                    (wrapped_desc, std::regex("\n"), "\n\t\t") + "\n";
            }
        }
        help_str += line;
    }
    return help_str;
}

void print_and_exit(std::string text, ConsoleOutputSupport& con) {
    printf("%s\n", text.c_str());
    con.detach();
    exit(0);
}

void init_logging(std::string loglevel) {
    auto severity = plog::severityFromString(loglevel.c_str());
    plog::init<plog::TxtFormatter>(severity, plog::streamStdOut);
}

void cli::setup(int argc, wchar_t* argv[], ConsoleOutputSupport& con) {
    std::string loglevel = "info";
    bool loglevel_waiting_for_value = false;
    if (argc > 1) {
        for (auto i = 1; i < argc; ++i) {
            auto warg = wstring_to_string(argv[i]);
            if (arg_equal(warg, options["help"].aliases)) {
                print_and_exit(generate_help_str(), con);
            } else if (arg_equal(warg, options["loglevel"].aliases)) {
                loglevel_waiting_for_value = true;
            } else {
                if (loglevel_waiting_for_value) {
                    bool found = false;
                    for (const auto& value :
                             options["loglevel"].possible_values) {
                        if (warg == value) {
                            loglevel = value;
                            found = true;
                        }
                    }
                    if (!found)
                        print_and_exit
                            (err_prefix + "argument " +
                             vector_join(options["loglevel"].aliases, "/") +
                             ": invalid choice: '" + warg +
                             "' (choose from " +
                             vector_join(options["loglevel"].possible_values,
                                         ", ") + ")", con);
                    loglevel_waiting_for_value = false;
                } else {
                    print_and_exit
                        (err_prefix + "unrecognised arguments: " +
                         wstring_to_string(vector_join_w(argc, argv, 1)), con);
                }
            }
        }
        if (loglevel_waiting_for_value) {
            print_and_exit
                (err_prefix + "argument " +
                 vector_join(options["loglevel"].aliases, "/") +
                 ": expected 1 argument", con);
        }
    }
    init_logging(loglevel);
    LOG_INFO << program_str;
}
