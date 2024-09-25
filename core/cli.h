#pragma once

#include <vector>
#include <string>
#include "utility/console_output_support.h"

namespace cli {
    struct Option {
        std::vector<std::string> aliases;
        std::string desc;
        bool takes_value = false;
        std::vector<std::string> possible_values = {};
    };
    void setup(int argc, wchar_t* argv[], ConsoleOutputSupport& con);
}
