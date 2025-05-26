#include <filesystem>
#include <format>
#include <fstream>
#include <iostream>
#include <iterator>
#include <optional>
#include <print>
#include <regex>
#include <time.h>

#include <cxxopts.hpp>

#include "HttpsRequest.hpp"
#include "InputDirectoryConfig.hpp"
#include "Utilities.ipp"

std::string DAY;
std::string YEAR;
bool FORCE_OVERWRITE{false};
bool DRY_RUN{false};
std::vector<std::string> CREATED_FILES;

bool DayIsValid()
{
    const auto day = util::StringTo<int>(DAY);
    return day >= 1 && day <= 31;
}

std::tm GetSystemTime()
{
    // https://stackoverflow.com/a/58153628
    const std::time_t t = std::time(nullptr);
    std::tm pTInfo;
#ifdef WIN32
    if(localtime_s(&pTInfo, &t) == nullptr)
#else
    if(localtime_r(&t, &pTInfo) == nullptr)
#endif
    {
        throw std::runtime_error("failed to get system time");
    }

    return pTInfo;
}

int GetCurrentYear()
{
    const auto systemTime = GetSystemTime();

    auto currentYear = 1900 + systemTime.tm_year;

    // AoC starts December 1st. If it's not December yet, use the previous year.
    if(systemTime.tm_mon < 11)
    {
        --currentYear;
    }

    return currentYear;
}

std::string GetCurrentYearString()
{
    return std::to_string(GetCurrentYear());
}

int GetCurrentDay()
{
    return GetSystemTime().tm_mday;
}

std::string GetCurrentDayString()
{
    return std::to_string(GetCurrentDay());
}

bool YearIsValid()
{
    const auto year = util::StringTo<int>(YEAR);
    return year >= 2015 && year <= GetCurrentYear();
}

bool TryCreateSolutionDirectory(const std::string_view rootPath)
{
    if(std::filesystem::exists(rootPath) && !FORCE_OVERWRITE)
    {
        std::println("Solution directory already exists");
        return false;
    }

    if(!DRY_RUN)
    {
        std::filesystem::create_directory(rootPath);
        return std::filesystem::exists(rootPath);
    }

    std::println("Creating solution directory '{}'", rootPath);
    return true;
}

bool CreateCMakeLists(const std::filesystem::path& x)
{
    const auto cmakeLists = x / "CMakeLists.txt";

    if(DRY_RUN)
    {
        std::println("Creating CMakeLists '{}'", cmakeLists.string());
        return true;
    }

#ifdef WIN32
    if(const auto contents = util::Parse("../CMakeLists.txt.in"); !contents.empty())
#else
    if(const auto contents = util::Parse("CMakeLists.txt.in"); !contents.empty())
#endif
    {
        if(std::ofstream ofs{cmakeLists}; ofs.is_open())
        {
            const std::regex re{"@DAY@"};
            ofs << std::regex_replace(contents, re, DAY);

            CREATED_FILES.push_back(cmakeLists.string());

            return true;
        }
    }

    return false;
}

bool CreateSourceFiles(const std::filesystem::path& x)
{
    const auto fullPath = x / std::format("{}.cpp", DAY);

    if(DRY_RUN)
    {
        std::println("Creating source files '{}'", fullPath.string());
        return true;
    }

#ifdef WIN32
    if(const auto contents = util::Parse("../Solution.cpp.in"); !contents.empty())
#else
    if(const auto contents = util::Parse("Solution.cpp.in"); !contents.empty())
#endif
    {
        if(std::ofstream ofs{fullPath}; ofs.is_open())
        {
            const std::regex re{"@DAY@"};
            ofs << std::regex_replace(contents, re, DAY);

            CREATED_FILES.push_back(fullPath.string());

            return true;
        }
    }

    return false;
}

bool DownloadInput()
{
    const auto fileName = std::format("{}/{}.txt", config::GetInputFilePath(), DAY);
    if(!FORCE_OVERWRITE && std::filesystem::exists(fileName))
    {
        std::println("Input file already exists: '{}'", fileName);
        return true;
    }

    HttpsRequest request;
    request.setUrl(std::format("https://adventofcode.com/{}/day/{}/input", YEAR, DAY));
    request.setContentType("text/plain");

    if(DRY_RUN)
    {
        // TODO: Add format support for HttpsRequest.
        std::println("Making HTTPS request for input...");
        return true;
    }

    if(const auto content = request())
    {
        if(std::ofstream ofs{config::GetInputFilePath() + "/" + DAY + ".txt"}; ofs.is_open())
        {
            ofs << *content;

            CREATED_FILES.push_back(fileName);

            return true;
        }
    }

    return false;
}

bool DownloadSampleInput()
{
    const auto fileName = std::format("{}/{}_sample.txt", config::GetInputFilePath(), DAY);
    if(!FORCE_OVERWRITE && std::filesystem::exists(fileName))
    {
        std::println("Input file already exists: '{}'", fileName);
        return true;
    }

    HttpsRequest request;
    request.setUrl(std::format("https://adventofcode.com/{}/day/{}", YEAR, DAY));
    request.setContentType("text/html");

    if(DRY_RUN)
    {
        // TODO: Add format support for HttpsRequest.
        std::println("Making HTTPS request for sample input...");
        return true;
    }

    if(const auto content = request())
    {
        // Beginning of sample input starts with "<pre><code>" and ends with "</code></pre>"
        // Ex:
        // <pre><code>A Y
        // B X
        // C Z
        // </code></pre>
        const std::string startTags = "<pre><code>";
        const auto beginPos = content->find(startTags) + startTags.size();
        const auto endTags = "</code></pre>";
        const auto size = content->find(endTags) - beginPos;

        if(std::ofstream ofs{fileName}; ofs.is_open())
        {
            ofs << content->substr(beginPos, size);

            CREATED_FILES.push_back(fileName);

            return true;
        }
    }

    return false;
}

int main(int argc, char** argv)
{
    cxxopts::Options options{
        "StartNewDay", "Create C++ stub code for new Advent of Code challenge and download input"};

    // TODO: Positional arguments are not showing up in help
    // clang-format off
    options.add_options()
        ("day", "The day number to use", cxxopts::value<std::string>()->default_value(GetCurrentDayString()))
        ("year", "The year to use", cxxopts::value<std::string>()->default_value(GetCurrentYearString()))
        ("dryrun", "Doesn't reach out to the network and doesn't touch the filesystem", cxxopts::value<bool>()->default_value("false"))
        ("f,force", "Force overwrite", cxxopts::value<bool>()->default_value("false"))
        ("h,help", "Shows this help message")
    ;
    // clang-format on

    options.parse_positional({"day", "year"});

    try
    {
        const auto result = options.parse(argc, argv);

        if(result.count("help"))
        {
            std::cout << options.help() << "\n";
            return EXIT_SUCCESS;
        }

        DRY_RUN = result["dryrun"].as<bool>();

        DAY = result["day"].as<std::string>();
        if(!DayIsValid())
        {
            std::println(std::cerr, "Error: '{}' must be between 1 and 31 inclusive", DAY);
            return EXIT_FAILURE;
        }

        YEAR = result["year"].as<std::string>();
        if(!YearIsValid())
        {
            std::println(std::cerr, "Error: '{}' must be between 2015 and current year inclusive",
                         YEAR);
            return EXIT_FAILURE;
        }

        FORCE_OVERWRITE = result["force"].as<bool>();

        const std::filesystem::path solutionsPath = config::GetSolutionsPath();
        if(!std::filesystem::exists(solutionsPath))
        {
            std::println(std::cerr, "Error: Could not find solutions path '{}'",
                         solutionsPath.string());
            return EXIT_FAILURE;
        }

        if(FORCE_OVERWRITE)
        {
            std::println("Overwriting day {}...", DAY);
        }
        else
        {
            std::println("Starting new day {}...", DAY);
        }

        const auto newDayPath = solutionsPath / DAY;
        if(!TryCreateSolutionDirectory(newDayPath.generic_string()))
        {
            std::println(std::cerr, "Could not create new subdirectory {}", newDayPath.string());
            std::cout << options.help() << "\n";
            std::println(std::cerr);

            return EXIT_FAILURE;
        }

        if(!CreateCMakeLists(newDayPath))
        {
            std::println(std::cerr, "Error: Could not create CMakeLists.txt for '{}'",
                         newDayPath.string());
            return EXIT_FAILURE;
        }

        if(!CreateSourceFiles(newDayPath))
        {
            std::println(std::cerr, "Error: Could not create source files for '{}'",
                         newDayPath.string());
            return EXIT_FAILURE;
        }

        if(!DownloadInput())
        {
            std::println(std::cerr, "Error: Could not download input");
            return EXIT_FAILURE;
        }

        if(!DownloadSampleInput())
        {
            // This is not a deal-breaker. Just grab it yourself ya bum.
            std::println(std::cerr, "Warning: Could not download sample input. Continuing...");
        }

        std::println("Files created:");
        std::ranges::copy(CREATED_FILES, std::ostream_iterator<std::string>(std::cout, "\n"));

        std::println();
        std::println("Re-run CMake to build the new day");

        return EXIT_SUCCESS;
    }
    catch(const cxxopts::exceptions::parsing& e)
    {
        std::println(std::cerr, "Error parsing option: {}", e.what());
        std::cout << options.help() << "\n";
    }
    catch(const cxxopts::exceptions::option_has_no_value& e)
    {
        std::println(std::cerr, "Error parsing required option: {}", e.what());
        std::cout << options.help() << "\n";
    }
    catch(const cxxopts::exceptions::specification& e)
    {
        std::println(std::cerr, "Error defining option specification: {}", e.what());
        std::println(std::cerr, "Please report this as a bug: "
                                "https://github.com/jonathondgebhardt/advent-of-code-cpp/issues");
    }
    catch(const cxxopts::exceptions::exception& e)
    {
        std::println(std::cerr, "Error: {}", e.what());
        std::cout << options.help() << "\n";
    }
    catch(const std::exception& e)
    {
        std::println(std::cerr, "Error: {}", e.what());
    }
    catch(...)
    {
        std::println(std::cerr, "Unknown error");
    }

    return EXIT_FAILURE;
}
