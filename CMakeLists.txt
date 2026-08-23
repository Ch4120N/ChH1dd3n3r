cmake_minimum_required(VERSION 3.20)
project(ChH1dd3n3r VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Force output to the root of the build directory for predictable packaging
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR})

option(STATIC_LINKING "Build static executable" OFF)

if(STATIC_LINKING)
    if(MSVC)
        # Static MSVC runtime (no VCRUNTIME DLL dependency)
        set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
    else()
        # Prefer static libraries (.a) and disable shared libraries
        set(CMAKE_FIND_LIBRARY_SUFFIXES .a ${CMAKE_FIND_LIBRARY_SUFFIXES})
        set(BUILD_SHARED_LIBS OFF)

        # On Linux, link libgcc and libstdc++ statically to reduce dependencies
        if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
            # Add -m32 if building for 32-bit x86
            if(CMAKE_SIZEOF_VOID_P EQUAL 4 AND CMAKE_SYSTEM_PROCESSOR MATCHES "i686|x86")
                add_link_options(-m32 -static-libgcc -static-libstdc++)
            else()
                add_link_options(-static-libgcc -static-libstdc++)
            endif()
        endif()
    endif()
endif()

# Force vcpkg to provide static versions of these libraries
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

install(TARGETS chh1dd3n3r RUNTIME DESTINATION bin)