file(GLOB_RECURSE sources "test.cpp")

add_executable(test ${sources})

target_link_libraries(test PRIVATE gtest_main gmock_main)