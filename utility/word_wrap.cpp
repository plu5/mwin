#include <iostream>
#include <sstream>

// rosettacode.org/wiki/Word_wrap
std::string wrap(const std::string &text, size_t line_length=72) {
    std::istringstream words(std::string(text.c_str()));
    std::ostringstream wrapped;
    std::string word;

    if (words >> word) {
        wrapped << word;
        size_t space_left = line_length - word.length();
        while (words >> word) {
            if (space_left < word.length() + 1) {
                wrapped << '\n' << word;
                space_left = line_length - word.length();
            } else {
                wrapped << ' ' << word;
                space_left -= word.length() + 1;
            }
        }
    }

    return wrapped.str();
}
