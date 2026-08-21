cmake_minimum_required(VERSION 3.21)

project(keypr-core LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(unofficial-sodium CONFIG REQUIRED)

add_library(keypr-core
    src/CryptoService.cpp
)

target_include_directories(keypr-core
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(keypr-core
    PRIVATE
        unofficial-sodium::sodium
)
