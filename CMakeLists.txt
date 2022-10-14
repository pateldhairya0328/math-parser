cmake_minimum_required(VERSION 3.22 FATAL_ERROR)

set(CMAKE_CXX_STANDARD 20)

project(math_parser VERSION 0.1 LANGUAGES C CXX)

# Include CPM for dependency management
include(cmake/CPM.cmake)

# Googletest for testing
CPMAddPackage(
  NAME googletest
  GITHUB_REPOSITORY google/googletest
  GIT_TAG release-1.12.1
  VERSION 1.12.1
  OPTIONS "INSTALL_GTEST OFF" "gtest_force_shared_crt"
  SOURCE_DIR ${PROJECT_SOURCE_DIR}/dependencies/googletest
)

# All targets need access to the public header files.
include_directories(include)

# Add tests
add_subdirectory(test)