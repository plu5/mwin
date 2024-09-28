#include "uid.h"
#include <combaseapi.h>
#include <stdexcept>
#include <algorithm>
#include <cstdint>
#include <vector>

using SplitValues = std::vector<int>;

template<typename T>
SplitValues split_nybbles(T number) {
    // https://codereview.stackexchange.com/a/30599/267629
    SplitValues values(2 * sizeof number);
    for (auto& v : values) {
        v = static_cast<int>(number & 0xF);
        number >>= 4;
    }
    std::reverse(values.begin(), values.end());
    return values;
}

void combine_values(SplitValues& a, SplitValues& b) {
    b.insert(b.end(), a.begin(), a.end());
}

std::string hex_to_b64(SplitValues hex) {
    const char* table="ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz"
        "0123456789" "-_";
    std::string res;
    const auto hex_size = hex.size();
    res.reserve // ceil((input_length * 4) / 6)
        (static_cast<size_t>(ceil((hex_size * static_cast<double>(4)) / 6)));
    for (size_t i = 0; i < hex_size; i += 3) {
        int d = hex[i];
        d <<= 4;
        if (size_t h = i+1; h < hex_size) {
            d += hex[h];
            d <<= 4;
            if (h += 1; h < hex_size) d += hex[h];
        }
        res.append(1, table[(d>>6)&63]);
        res.append(1, table[d&63]);
    }
    return res.c_str();
}

SplitValues guid_to_splitvalues(GUID guid) {
    auto values = split_nybbles(guid.Data1);
    auto values_ = split_nybbles(guid.Data2);
    combine_values(values_, values);
    values_ = split_nybbles(guid.Data3);
    combine_values(values_, values);
    for (auto i = 0; i < 8; i++) {
        values_ = split_nybbles(guid.Data4[i]);
        combine_values(values_, values);
    }
    return values;
}

std::string uid() {
    GUID guid;
    if (CoCreateGuid(&guid) != S_OK)
        throw std::runtime_error("GUID creation failed");
    return hex_to_b64(guid_to_splitvalues(guid));
}
