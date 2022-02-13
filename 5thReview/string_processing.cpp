#include "string_processing.h"

VectorStringView SplitIntoWords(std::string_view str) {
    VectorStringView result;
    while (true) {
        const auto space = str.find(' ');
        result.push_back(str.substr(0, space));
        if (space == str.npos) {
            break;
        } else {
            str.remove_prefix(space + 1);
        }
    }
    return result;
}
