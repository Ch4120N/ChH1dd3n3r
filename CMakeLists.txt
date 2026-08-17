cmake_minimum_required(VERSION 3.20)
project(ChH1dd3n3r VERSION 3.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(OpenSSL REQUIRED)
find_package(ZLIB REQUIRED)

add_executable(chh1dd3n3r
    src/main.cpp
    src/magic.cpp
    src/crypto_utils.cpp
    src/tar_utils.cpp
    src/logger.cpp
    src/spinner.cpp
    src/password_utils.cpp
    src/help_center.cpp
    src/container.cpp
    src/engine.cpp
    src/cli.cpp
)

target_include_directories(chh1dd3n3r PRIVATE include)
target_link_libraries(chh1dd3n3r PRIVATE OpenSSL::Crypto ZLIB::ZLIB)

if(MSVC)
    target_compile_options(chh1dd3n3r PRIVATE /utf-8)
else()
    target_compile_options(chh1dd3n3r PRIVATE -Wall -Wextra -Wpedantic -finput-charset=utf-8 -fexec-charset=utf-8)
endif()

install(TARGETS chh1dd3n3r RUNTIME DESTINATION bin)