#pragma once
#include <vector>
#include <string>

inline void split(const std::string& str, char const delimiter, std::vector<std::string>& split_result) {
    size_t start = 0;
    size_t end = str.find(delimiter);

    while (end != std::string::npos) {
        split_result.push_back(str.substr(start, end - start));
        start = end + 1;
        end = str.find(delimiter, start);
    }

    split_result.push_back(str.substr(start));
}

inline void split(const std::string_view& str, char const delimiter, std::vector<std::string_view>& split_result) {
    size_t start = 0;
    size_t end = str.find(delimiter);

    while (end != std::string::npos) {
        split_result.push_back(str.substr(start, end - start));
        start = end + 1;
        end = str.find(delimiter, start);
    }

    split_result.push_back(str.substr(start));
}