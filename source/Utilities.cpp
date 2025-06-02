#include <format>
#include <fstream>
#include <iostream>
#include <print>
#include <sstream>
#include <string>
#include <string_view>
#include <vector>

#include "Utilities.hpp"

#include "InputDirectoryConfig.hpp"

auto util::get_input_file(const std::string_view stem) -> std::string
{
  return std::format("{}/{}", config::input_file_path, stem);
}

auto util::parse_to_container(const std::string_view file_path)
    -> std::vector<std::string>
{
  if (std::ifstream ifs {file_path.data()}; ifs.is_open()) {
    std::vector<std::string> contents;

    for (std::string line; std::getline(ifs, line);) {
      contents.push_back(line);
    }

    // Add the trailing new line to preserve input representation.
    contents.emplace_back();
    return contents;
  }

  std::println(std::cerr, "Could not open '{}'", file_path);

  return {};
}

auto util::parse(const std::string_view file_path) -> std::string
{
  if (std::ifstream ifs {file_path.data()}; ifs.is_open()) {
    std::stringstream stream;

    for (std::string line; std::getline(ifs, line);) {
      stream << std::format("{}\n", line);
    }

    // Add the trailing new line to preserve input representation.
    stream << '\n';

    return stream.str();
  }

  std::println(std::cerr, "Could not open '{}'", file_path);

  return {};
}

auto util::split(const std::string_view string, const char delimiter)
    -> std::vector<std::string>
{
  std::vector<std::string> tokens;

  std::stringstream stream {string.data()};
  std::string line;
  while (std::getline(stream, line, delimiter)) {
    tokens.push_back(line);
  }

  return tokens;
}
