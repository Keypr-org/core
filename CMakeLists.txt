cmake_minimum_required(VERSION 3.21)

project(keypr-core LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(unofficial-sodium CONFIG REQUIRED)
find_package(GTest CONFIG REQUIRED)

# ----------------------------------------------
#           Core Lib Compilation
# ----------------------------------------------

add_library(keypr-core
    src/CryptoService.cpp
)

target_include_directories(keypr-core
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(keypr-core
    PUBLIC
        unofficial-sodium::sodium
)

# ----------------------------------------------
#           Gtest Tests Compilation
# ----------------------------------------------

include(CTest)
enable_testing()

add_executable(core_tests
    tests/test_CryptoService.cpp
)

target_link_libraries(core_tests
    PRIVATE
        keypr-core
        GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(core_tests)

