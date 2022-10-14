file(GLOB_RECURSE sources "test.cpp")

add_executable(math_parser_test ${sources})

target_link_libraries(math_parser_test PRIVATE gtest_main gmock_main math_parser)