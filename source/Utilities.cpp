#include <fstream>
#include <iostream>

#include "InputDirectoryConfig.hpp"
#include "Utilities.ipp"

std::string util::GetInputFile(const std::string_view stem)
{
    return std::format("{}/{}", config::GetInputFilePath(), stem);
}

std::vector<std::string> util::ParseToContainer(const std::string_view filePath)
{

    if(std::ifstream ifs{filePath.data()}; ifs.is_open())
    {
        std::vector<std::string> contents;

        for(std::string line; std::getline(ifs, line);)
        {
            contents.push_back(line);
        }

        // Add the trailing new line to preserve input representation.
        contents.emplace_back();
        return contents;
    }

    std::println(std::cerr, "Could not open '{}'", filePath);

    return {};
}

std::string util::Parse(const std::string_view filePath)
{
    if(std::ifstream ifs{filePath.data()}; ifs.is_open())
    {
        std::stringstream ss;

        for(std::string line; std::getline(ifs, line);)
        {
            ss << std::format("{}\n", line);
        }

        // Add the trailing new line to preserve input representation.
        ss << '\n';

        return ss.str();
    }

    std::println(std::cerr, "Could not open '{}'", filePath);

    return {};
}

std::vector<std::string> util::Split(const std::string_view x, const char delimiter)
{
    std::vector<std::string> tokens;

    std::stringstream ss{x.data()};
    std::string s;
    while(std::getline(ss, s, delimiter))
    {
        tokens.push_back(s);
    }

    return tokens;
}
