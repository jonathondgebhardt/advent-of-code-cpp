#pragma once

#include <algorithm>
#include <sstream>
#include <string>
#include <vector>

namespace util
{
    std::string GetInputFile(std::string_view stem);

    std::vector<std::string> ParseToContainer(std::string_view filePath);

    std::string Parse(std::string_view filePath);

    template <class T> T StringTo(std::string_view x)
    {
        T result;
        std::stringstream ss{x.data()};
        ss >> result;

        return result;
    }

    template <class T> std::vector<T> ContainerTo(const std::vector<std::string>& x)
    {
        std::vector<T> converted;
        std::ranges::transform(x, std::back_inserter(converted),
                               [](const auto& y) { return StringTo<T>(y); });

        return converted;
    }

    std::vector<std::string> Split(std::string_view x, char delimiter = ' ');
}
