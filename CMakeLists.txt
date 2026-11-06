
cmake_minimum_required(VERSION 3.15)

project(ComputerAlgebraSystem VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(FetchContent)

# Boost.Multiprecision
FetchContent_Declare(
    boost_multiprecision
    URL https://github.com/boostorg/multiprecision/archive/refs/tags/Boost_1_92_0.zip
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)

FetchContent_MakeAvailable(boost_multiprecision)

# Source files
file(GLOB_RECURSE SRC_FILES CONFIGURE_DEPENDS "src/*.cpp")
file(GLOB_RECURSE DEMO_FILES CONFIGURE_DEPENDS "demo/*.cpp")
file(GLOB_RECURSE TEST_FILES CONFIGURE_DEPENDS "tests/*.cpp")

# Core library
add_library(cas_core STATIC ${SRC_FILES})

target_include_directories(cas_core PUBLIC src)

target_link_libraries(cas_core PUBLIC Boost::multiprecision)

# Demo executable
add_executable(demo ${DEMO_FILES})
target_link_libraries(demo PRIVATE cas_core)

# Tests executable
add_executable(tests ${TEST_FILES})
target_link_libraries(tests PRIVATE cas_core)
