#pragma once

#include <algorithm>
#include <span>
#include <sstream>
#include <string>
#include <vector>

namespace util
{
auto get_input_file(std::string_view stem) -> std::string;

auto parse_to_container(std::string_view file_path) -> std::vector<std::string>;

auto parse(std::string_view file_path) -> std::string;

template<class T>
auto string_to(std::string_view string) -> T
{
    T result;
    std::stringstream stream{string.data()};
    stream >> result;

    return result;
}

template<class T>
auto container_to(const std::span<const std::string>& container)
    -> std::vector<T>
{
    std::vector<T> converted;
    std::ranges::transform(container,
                           std::back_inserter(converted),
                           [](const auto& string)
                           { return string_to<T>(string); });

    return converted;
}

auto split(std::string_view string,
           char delimiter = ' ') -> std::vector<std::string>;
}  // namespace util
