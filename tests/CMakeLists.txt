find_package(GTest CONFIG REQUIRED)

add_executable(core_tests
    test_CryptoService.cpp
    test_FileHandler.cpp
)

target_link_libraries(core_tests
    PRIVATE
        keypr-core
        GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(core_tests)
