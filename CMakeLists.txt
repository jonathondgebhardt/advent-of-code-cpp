cmake_minimum_required(VERSION 3.14)

include(cmake/prelude.cmake)

project(
    advent-of-code-cpp
    VERSION 0.1.0
    DESCRIPTION "Short description"
    HOMEPAGE_URL "https://example.com/"
    LANGUAGES CXX
)

include(cmake/project-is-top-level.cmake)
include(cmake/variables.cmake)

# ---- Declare library ----

# Generate a file that allows source code to use paths in the project like "inputs".
get_filename_component(INPUT_FILE_PATH "inputs" REALPATH BASE_DIR "${CMAKE_CURRENT_SOURCE_DIR}")
set(SOLUTIONS_PATH "${CMAKE_CURRENT_SOURCE_DIR}/solutions")
configure_file(source/InputDirectoryConfig.hpp.in ${PROJECT_BINARY_DIR}/source/InputDirectoryConfig.hpp)

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/source/CMakeLists.txt.in DESTINATION ${PROJECT_BINARY_DIR}/source)
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/source/Solution.cpp.in DESTINATION ${PROJECT_BINARY_DIR}/source)

add_library(
    advent-of-code-cpp_lib STATIC
    source/Utilities.cpp
)

target_include_directories(
    advent-of-code-cpp_lib ${warning_guard}
    PUBLIC
    "\$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/source>"
)

target_include_directories(
    advent-of-code-cpp_lib ${warning_guard}
    PUBLIC
    "\$<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/source>"
)

target_compile_features(advent-of-code-cpp_lib PUBLIC cxx_std_23)

# ---- Declare executable ----

add_executable(advent-of-code-cpp_exe
    source/HttpsRequest.cpp
    source/StartNewDay.cpp
)
add_executable(advent-of-code-cpp::exe ALIAS advent-of-code-cpp_exe)

set_property(TARGET advent-of-code-cpp_exe PROPERTY OUTPUT_NAME advent-of-code-cpp)

target_compile_features(advent-of-code-cpp_exe PRIVATE cxx_std_23)

find_package(CURL CONFIG REQUIRED)
find_package(cxxopts CONFIG REQUIRED)
target_link_libraries(advent-of-code-cpp_exe PRIVATE 
    advent-of-code-cpp_lib
    CURL::libcurl 
    cxxopts::cxxopts 
)

# ---- Install rules ----

if(NOT CMAKE_SKIP_INSTALL_RULES)
  include(cmake/install-rules.cmake)
endif()

# ---- Developer mode ----

if(NOT advent-of-code-cpp_DEVELOPER_MODE)
  return()
elseif(NOT PROJECT_IS_TOP_LEVEL)
  message(
      AUTHOR_WARNING
      "Developer mode is intended for developers of advent-of-code-cpp"
  )
endif()

include(cmake/dev-mode.cmake)
