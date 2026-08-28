find_package(GTest CONFIG REQUIRED)

add_executable(core_tests
    test_FileHandler.cpp
    test_CryptoService.cpp
    test_VaultHeader.cpp
    test_RawVault.cpp
    test_VaultSession.cpp
)

target_link_libraries(core_tests
    PRIVATE
        keypr-core
        GTest::gmock
        GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(core_tests)
