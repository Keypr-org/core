cmake_minimum_required(VERSION 3.21)

project(keypr-core LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(CTest)

find_package(unofficial-sodium CONFIG REQUIRED)
find_package(nlohmann_json 3.12.0 REQUIRED)

add_library(keypr-core
    src/FileHandler.cpp
    src/CryptoService.cpp
    src/VaultHeader.cpp
    src/RawVault.cpp
    src/VaultSession.cpp
    src/entities/Category.cpp
    src/entities/CreditCard.cpp
    src/entities/DatedItem.cpp
    src/entities/Entry.cpp
    src/entities/Item.cpp
    src/entities/Persona.cpp
    src/entities/Website.cpp
    src/entities/Wifi.cpp
    src/VaultRepository.cpp
)

target_include_directories(keypr-core
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(keypr-core
    PUBLIC
        unofficial-sodium::sodium
        nlohmann_json::nlohmann_json
)

target_compile_definitions(keypr-core
    PRIVATE
        SODIUM_STATIC=1
)

if(BUILD_TESTING)
  add_subdirectory(tests)
endif()
