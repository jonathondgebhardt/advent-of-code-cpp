#include <algorithm>
#include <cstdlib>
#include <ctime>
#include <filesystem>
#include <format>
#include <fstream>
#include <iostream>
#include <iterator>
#include <optional>
#include <print>
#include <regex>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

#include <cxxopts.hpp>

#include "HttpsRequest.hpp"
#include "InputDirectoryConfig.hpp"
#include "Utilities.hpp"

namespace
{
std::string DAY;
std::string YEAR;
bool FORCE_OVERWRITE{false};
bool NO_TOUCH{false};
bool NO_DOWNLOAD{false};
std::vector<std::string> CREATED_FILES;

auto day_is_valid() -> bool
{
    const auto day = util::string_to<int>(DAY);
    constexpr auto last_day_in_december = 31;
    return day >= 1 && day <= last_day_in_december;
}

auto get_system_time() -> std::tm
{
    // https://stackoverflow.com/a/58153628
    const std::time_t time = std::time(nullptr);
    std::tm p_t_info{};
#ifdef WIN32
    if (localtime_s(&pTInfo, &t) == nullptr)
#else
    if (localtime_r(&time, &p_t_info) == nullptr)
#endif
    {
        throw std::runtime_error("failed to get system time");
    }

    return p_t_info;
}

auto get_current_year() -> int
{
    const auto system_time = get_system_time();

    constexpr auto offset = 1900;
    auto current_year = offset + system_time.tm_year;

    // AoC starts December 1st. If it's not December yet, use the previous year.
    if (constexpr auto november = 11; system_time.tm_mon < november) {
        --current_year;
    }

    return current_year;
}

auto get_current_year_string() -> std::string
{
    return std::to_string(get_current_year());
}

auto get_current_day() -> int
{
    return get_system_time().tm_mday;
}

auto get_current_day_string() -> std::string
{
    return std::to_string(get_current_day());
}

auto year_is_valid() -> bool
{
    const auto year = util::string_to<int>(YEAR);
    constexpr auto first_year_of_aoc = 2015;
    return year >= first_year_of_aoc && year <= get_current_year();
}

auto try_create_solution_directory(const std::string_view root_path) -> bool
{
    if (std::filesystem::exists(root_path) && !FORCE_OVERWRITE) {
        std::println("Solution directory already exists");
        return false;
    }

    if (!NO_TOUCH) {
        std::filesystem::create_directory(root_path);
        return std::filesystem::exists(root_path);
    }

    std::println("Creating solution directory '{}'", root_path);
    return true;
}

auto create_cmake_lists(const std::filesystem::path& path) -> bool
{
    const auto cmake_lists = path / "CMakeLists.txt";

    if (NO_TOUCH) {
        std::println("Creating CMakeLists '{}'", cmake_lists.string());
        return true;
    }

#ifdef WIN32
    const auto* const infile = "../CMakeLists.txt.in";
#else
    const auto* const infile = "CMakeLists.txt.in";
#endif

    if (const auto contents = util::parse(infile); !contents.empty()) {
        if (std::ofstream ofs{cmake_lists}; ofs.is_open()) {
            const std::regex exp{"@DAY@"};
            ofs << std::regex_replace(contents, exp, DAY);

            CREATED_FILES.push_back(cmake_lists.string());

            return true;
        }
    }

    return false;
}

// TODO: Use an in-file for this?
auto create_source_files(const std::filesystem::path& path) -> bool
{
    const auto full_path = path / std::format("{}.cpp", DAY);

    if (NO_TOUCH) {
        std::println("Creating source files '{}'", full_path.string());
        return true;
    }

#ifdef WIN32
    const auto* const infile = "../Solution.cpp.in";
#else
    const auto* const infile = "Solution.cpp.in";
#endif

    if (const auto contents = util::parse(infile); !contents.empty()) {
        if (std::ofstream ofs{full_path}; ofs.is_open()) {
            const std::regex exp{"@DAY@"};
            ofs << std::regex_replace(contents, exp, DAY);

            CREATED_FILES.push_back(full_path.string());

            return true;
        }
    }

    return false;
}

auto download_input() -> bool
{
    const auto file_name =
        std::format("{}/{}.txt", config::input_file_path, DAY);
    if (!FORCE_OVERWRITE && std::filesystem::exists(file_name)) {
        std::println("Input file already exists: '{}'", file_name);
        return true;
    }

    const https_request request;
    request.set_url(
        std::format("https://adventofcode.com/{}/day/{}/input", YEAR, DAY));
    request.set_content_type("text/plain");

    if (NO_DOWNLOAD) {
        // TODO: Add format support for HttpsRequest.
        std::println("Making HTTPS request for input...");
        return true;
    }

    if (const auto content = request()) {
        if (std::ofstream ofs{
                std::format("{}/{}.txt", config::input_file_path, DAY)};
            ofs.is_open())
        {
            ofs << *content;

            CREATED_FILES.push_back(file_name);

            return true;
        }
    }

    return false;
}

auto download_sample_input() -> bool
{
    const auto file_name =
        std::format("{}/{}_sample.txt", config::input_file_path, DAY);
    if (!FORCE_OVERWRITE && std::filesystem::exists(file_name)) {
        std::println("Input file already exists: '{}'", file_name);
        return true;
    }

    https_request request;
    request.set_url(
        std::format("https://adventofcode.com/{}/day/{}", YEAR, DAY));
    request.set_content_type("text/html");

    if (NO_DOWNLOAD) {
        // TODO: Add format support for HttpsRequest.
        std::println("Making HTTPS request for sample input...");
        return true;
    }

    if (const auto content = request()) {
        // Beginning of sample input starts with "<pre><code>" and ends with
        // "</code></pre>" Ex: <pre><code>A Y B X C Z
        // </code></pre>
        const std::string start_tags = "<pre><code>";
        const auto begin_pos = content->find(start_tags) + start_tags.size();
        const auto* const end_tags = "</code></pre>";
        const auto size = content->find(end_tags) - begin_pos;

        if (std::ofstream ofs{file_name}; ofs.is_open()) {
            ofs << content->substr(begin_pos, size);

            CREATED_FILES.push_back(file_name);

            return true;
        }
    }

    return false;
}
}  // namespace

auto main(int argc, char** argv) -> int
{
    cxxopts::Options options{"StartNewDay",
                           "Create C++ stub code for new Advent of Code "
                           "challenge and download input"};

    // TODO: Positional arguments are not showing up in help
    // clang-format off
    options.add_options()
        ("day", "The day number to use", cxxopts::value<std::string>()->default_value(get_current_day_string()))
        ("year", "The year to use", cxxopts::value<std::string>()->default_value(get_current_year_string()))
        ("f,force", "Force overwrite", cxxopts::value<bool>()->default_value("false"))
        ("dry-run", "Same as --no-download and --no-touch", cxxopts::value<bool>())
        ("no-download", "Don't reach out to the network", cxxopts::value<bool>()->default_value("false"))
        ("no-touch", "Don't touch the filesystem", cxxopts::value<bool>()->default_value("false"))
        ("h,help", "Shows this help message")
    ;
    // clang-format on

    options.parse_positional({"day", "year"});

    try {
        const auto result = options.parse(argc, argv);

        if (result.count("help") != 0U) {
            std::cout << options.help() << "\n";
            return EXIT_SUCCESS;
        }

        if (result.count("dry-run") == 1U) {
            NO_TOUCH = true;
            NO_DOWNLOAD = true;

            // TODO: Log a warning if either of the others are present?
            if (result.count("no-touch") > 0 || result.count("no-download") > 0)
            {
                std::println(
                    "Warning: --no-touch or --no-download passed with "
                    "--dry-run");
                std::println("Warning: --dry-run overrides");
            }
        } else {
            NO_TOUCH = result["no-touch"].as<bool>();
            NO_DOWNLOAD = result["no-download"].as<bool>();
        }

        DAY = result["day"].as<std::string>();
        if (!day_is_valid()) {
            std::println(std::cerr,
                         "Error: '{}' must be between 1 and 31 inclusive",
                         DAY);
            return EXIT_FAILURE;
        }

        YEAR = result["year"].as<std::string>();
        if (!year_is_valid()) {
            std::println(
                std::cerr,
                "Error: '{}' must be between 2015 and current year inclusive",
                YEAR);
            return EXIT_FAILURE;
        }

        FORCE_OVERWRITE = result["force"].as<bool>();

        const std::filesystem::path solutions_path = config::solutions_path;
        if (!std::filesystem::exists(solutions_path)) {
            std::println(std::cerr,
                         "Error: Could not find solutions path '{}'",
                         solutions_path.string());
            return EXIT_FAILURE;
        }

        if (FORCE_OVERWRITE) {
            std::println("Overwriting day {}...", DAY);
        } else {
            std::println("Starting new day {}...", DAY);
        }

        const auto new_day_path = solutions_path / DAY;
        if (!try_create_solution_directory(new_day_path.generic_string())) {
            std::println(std::cerr,
                         "Could not create new subdirectory {}",
                         new_day_path.string());
            std::cout << options.help() << "\n";
            std::println(std::cerr);

            return EXIT_FAILURE;
        }

        if (!create_cmake_lists(new_day_path)) {
            std::println(std::cerr,
                         "Error: Could not create CMakeLists.txt for '{}'",
                         new_day_path.string());
            return EXIT_FAILURE;
        }

        if (!create_source_files(new_day_path)) {
            std::println(std::cerr,
                         "Error: Could not create source files for '{}'",
                         new_day_path.string());
            return EXIT_FAILURE;
        }

        if (!download_input()) {
            std::println(std::cerr, "Error: Could not download input");
            return EXIT_FAILURE;
        }

        if (!download_sample_input()) {
            // This is not a deal-breaker. Just grab it yourself ya bum.
            std::println(
                std::cerr,
                "Warning: Could not download sample input. Continuing...");
        }

        std::println("Files created:");
        std::ranges::copy(CREATED_FILES,
                          std::ostream_iterator<std::string>(std::cout, "\n"));

        std::println();
        std::println("Re-run CMake to build the new day");

        return EXIT_SUCCESS;
    } catch (const cxxopts::exceptions::parsing& e) {
        std::println(std::cerr, "Error parsing option: {}", e.what());
        std::cout << options.help() << "\n";
    } catch (const cxxopts::exceptions::option_has_no_value& e) {
        std::println(std::cerr, "Error parsing required option: {}", e.what());
        std::cout << options.help() << "\n";
    } catch (const cxxopts::exceptions::specification& e) {
        std::println(
            std::cerr, "Error defining option specification: {}", e.what());
        std::println(
        std::cerr,
        "Please report this as a bug: "
        "https://github.com/jonathondgebhardt/advent-of-code-cpp/issues");
    } catch (const cxxopts::exceptions::exception& e) {
        std::println(std::cerr, "Error: {}", e.what());
        std::cout << options.help() << "\n";
    } catch (const std::exception& e) {
        std::println(std::cerr, "Error: {}", e.what());
    } catch (...) {
        std::println(std::cerr, "Unknown error");
    }

    return EXIT_FAILURE;
}
