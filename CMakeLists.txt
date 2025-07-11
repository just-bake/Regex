cmake_minimum_required(VERSION 3.31)
project(cregex LANGUAGES C)

set(CMAKE_C_STANDARD 99) # Sets the C standard to C99

# Options to build extras
option(CREGEX_BUILD_TESTS "Build unit tests" OFF)
option(CREGEX_BUILD_EXAMPLES "Build usage examples" OFF)

add_library(cregex STATIC src/cregex.c)
target_include_directories(cregex PUBLIC include)

# Tests
if(CREGEX_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

# Examples
if(CREGEX_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()