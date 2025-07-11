cmake_minimum_required(VERSION 3.31)
project(regex LANGUAGES C)

set(CMAKE_C_STANDARD 99) # Sets the C standard to C99

# Options to build extras
option(REGEX_BUILD_TESTS "Build unit tests" OFF)
option(REGEX_BUILD_EXAMPLES "Build usage examples" OFF)

add_library(${PROJECT_NAME} STATIC src/regex.c)
target_include_directories(${PROJECT_NAME} PUBLIC include)

# Tests
if (REGEX_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif ()

# Examples
if (REGEX_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif ()