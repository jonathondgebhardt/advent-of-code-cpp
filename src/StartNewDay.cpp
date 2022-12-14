#include <InputDirectoryConfig.ipp>

#include <filesystem>
#include <fstream>
#include <iostream>
#include <optional>

void showUsage(const std::string& x)
{
    std::cout << "Usage: " << x << " NEW_DAY_SUBDIRECTORY_NAME [-f]\n";
    std::cout << "       NEW_DAY_SUBDIRECTORY_NAME Name of new day\n";
    std::cout << "       -f Force overwrite\n";
}

bool tryCreateSolutionDirectory(const std::string& rootPath, bool forceOverwrite)
{
    if(std::filesystem::exists(rootPath) && !forceOverwrite)
    {
        return false;
    }

    std::filesystem::create_directory(rootPath);
    return std::filesystem::exists(rootPath);
}

bool createCMakeLists(const std::filesystem::path& x)
{
    const auto cmakeLists = x / "CMakeLists.txt";
    std::ofstream ofs(cmakeLists);
    if(ofs.is_open())
    {
        ofs << "# File auto-generated by StartNewDay\n";
        ofs << "cmake_minimum_required(VERSION 3.0)\n";

        const auto dayNumber = std::string(x.filename());
        ofs << "project(" << dayNumber << " CXX)\n";
        ofs << "\n";

        ofs << "set(SOURCE_H\n";
        ofs << "    ACSolver_" << dayNumber << ".ipp\n";
        ofs << "    Utilities.ipp\n";
        ofs << ")\n";
        ofs << "\n";

        ofs << "set(SOURCE\n";
        ofs << "    ACSolver_" << dayNumber << ".cpp\n";
        ofs << ")\n";
        ofs << "\n";

        ofs << "add_executable(${PROJECT_NAME} ${SOURCE_H} ${SOURCE})\n";
        ofs << "target_link_libraries(${PROJECT_NAME} AOC)\n";

        return true;
    }

    return false;
}

bool createSourceFiles(const std::filesystem::path& x)
{
    const auto dayNumber = std::string(x.filename());

    {
        std::ofstream ofs(x / "Utilities.ipp");
        if(!ofs.is_open())
        {
            return false;
        }

        ofs << "#pragma once\n";

        ofs << "\n";

        ofs << "// File auto-generated by StartNewDay\n";

        ofs << "\n";

        ofs << "#include <Export.hpp>\n";
        ofs << "#include <Utilities.ipp>\n";

        ofs << "\n";

        ofs << "namespace util::" << dayNumber << "\n";
        ofs << "{\n";
        ofs << "}\n";
    }

    const auto className = std::string("ACSolver_") + dayNumber;

    {
        std::ofstream ofs(x / (className + std::string(".ipp")));
        if(!ofs.is_open())
        {
            return false;
        }

        ofs << "#pragma once\n";
        ofs << "\n";

        ofs << "#include <ACSolver.ipp>\n";
        ofs << "#include <EntryPoint.ipp>\n";
        ofs << "#include <Utilities.ipp>\n";
        ofs << "\n";

        ofs << "struct " << className << " : public ACSolver\n";
        ofs << "{\n";
        ofs << "    int64_t solvePartOne() override;\n";
        ofs << "    int64_t solvePartTwo() override;\n";
        ofs << "};\n";
        ofs << "\n";

        ofs << "std::unique_ptr<ACSolver> CreateSolver(bool useSample)\n";
        ofs << "{\n";
        ofs << "    const auto inputFile = useSample ? \"" << dayNumber << "_sample.txt\""
            << " : \"" << dayNumber << ".txt\";\n";
        ofs << "    auto solver = std::make_unique<" << className << ">();\n";
        ofs << "    solver->input = util::Parse(util::GetInputFile(inputFile));\n";
        ofs << "    return std::move(solver);\n";
        ofs << "}\n";
        ofs << "\n";
    }

    {
        std::ofstream ofs(x / (className + std::string(".cpp")));
        if(!ofs.is_open())
        {
            return false;
        }

        ofs << "#include \"" << className << ".ipp\"\n";
        ofs << "\n";

        ofs << "int64_t " << className << "::solvePartOne()\n";
        ofs << "{\n";
        ofs << "    return ACSolver::solvePartOne();\n";
        ofs << "}\n";
        ofs << "\n";

        ofs << "int64_t " << className << "::solvePartTwo()\n";
        ofs << "{\n";
        ofs << "    return ACSolver::solvePartTwo();\n";
        ofs << "}\n";
        ofs << "\n";
    }

    return true;
}

void showNewDayContents(const std::filesystem::path& x, size_t indent = 1)
{
    for(const auto& item : std::filesystem::directory_iterator(x))
    {
        for(size_t i = 0; i < indent; ++i)
        {
            std::cout << "\t";
        }

        std::cout << item << "\n";

        if(std::filesystem::is_directory(item))
        {
            showNewDayContents(item, ++indent);
            --indent;
        }
    }
}

int main(int argc, char* argv[])
{
    const auto applicationName = std::string(argv[0]);

    // TODO: Add command line argument for help
    // TODO: Add command line argument for dry-run
    if(argc < 2)
    {
        showUsage(applicationName);
        return EXIT_FAILURE;
    }

    std::filesystem::path solutionsPath(config::GetSolutionsPath());
    if(!std::filesystem::exists(solutionsPath))
    {
        std::cerr << "Could not find solutions path " << solutionsPath << "\n";
        return EXIT_FAILURE;
    }

    constexpr size_t newDayIndex = 1;
    const auto newDay = std::string(argv[newDayIndex]);

    constexpr size_t forceOverwriteIndex = 2;
    auto forceOverwrite = false;

    if(argc == 3 && std::string(argv[forceOverwriteIndex]) == "-f")
    {
        forceOverwrite = true;
    }

    const auto newDayPath = solutionsPath / newDay;
    if(!tryCreateSolutionDirectory(newDayPath, forceOverwrite))
    {
        std::cerr << "Could not create new subdirectory " << newDayPath << "\n";
        std::cerr << "Check write permissions or if subdirectory already exists\n";
        std::cerr << "\n";
        showUsage(applicationName);

        return EXIT_FAILURE;
    }

    if(forceOverwrite)
    {
        std::cout << "Overwriting day: " << newDay << "...\n";
    }
    else
    {
        std::cout << "Starting new day: " << newDay << "...\n";
    }

    if(!createCMakeLists(newDayPath))
    {
        std::cerr << "Could not create CMakeLists.txt for " << newDayPath << "\n";
        return EXIT_FAILURE;
    }

    if(!createSourceFiles(newDayPath))
    {
        std::cerr << "Could not create source files for " << newDayPath << "\n";
        return EXIT_FAILURE;
    }

    std::cout << "Files created:\n";
    showNewDayContents(newDayPath);
    std::cout << "\n";

    std::cout << "Be sure to add new directory to root solutions CMakeLists.txt and re-run CMake\n";

    return EXIT_SUCCESS;
}