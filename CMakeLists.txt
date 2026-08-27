cmake_minimum_required(VERSION 3.21)

project(keypr-core LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(CTest)

find_package(unofficial-sodium CONFIG REQUIRED)

add_library(keypr-core
    src/FileHandler.cpp
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

target_compile_definitions(keypr-core
    PRIVATE
        SODIUM_STATIC=1
)

if(BUILD_TESTING)
  add_subdirectory(tests)
endif()
