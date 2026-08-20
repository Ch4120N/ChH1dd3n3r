cmake_minimum_required(VERSION 3.20)
project(ChH1dd3n3r VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(STATIC_LINKING "Build static executable" OFF)

if(STATIC_LINKING)
    if(MSVC)
        set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
    else()
        set(CMAKE_FIND_LIBRARY_SUFFIXES .a ${CMAKE_FIND_LIBRARY_SUFFIXES})
        set(BUILD_SHARED_LIBS OFF)
        if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
            add_link_options(-static-libgcc -static-libstdc++)
        endif()
    endif()
endif()

# Prefer static libraries for OpenSSL and zlib
set(OPENSSL_USE_STATIC_LIBS TRUE)
set(ZLIB_USE_STATIC_LIBS TRUE)

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

if(STATIC_LINKING AND MSVC)
    set_property(TARGET chh1dd3n3r PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()

install(TARGETS chh1dd3n3r RUNTIME DESTINATION bin)