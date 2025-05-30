# Parent project does not export its library target, so this CML implicitly
# depends on being added from it, i.e. the testing is done only from the build
# tree and is not feasible from an install location

project(advent-of-code-cppTests LANGUAGES CXX)

# ---- Dependencies ----

find_package(Catch2 REQUIRED)
include(Catch)

# ---- Tests ----

add_executable(advent-of-code-cpp_test source/advent-of-code-cpp_test.cpp)
target_link_libraries(
    advent-of-code-cpp_test PRIVATE
    advent-of-code-cpp_lib
    Catch2::Catch2WithMain
)
target_compile_features(advent-of-code-cpp_test PRIVATE cxx_std_20)

catch_discover_tests(advent-of-code-cpp_test)

# ---- End-of-file commands ----

add_folders(Test)
