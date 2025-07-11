add_executable(regex_tests
        regex_tests.c
)

target_link_libraries(regex_tests PRIVATE regex)
add_test(NAME regex_tests COMMAND regex_tests)