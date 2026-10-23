cmake_minimum_required(VERSION 3.25)

project(BFCL
    VERSION 0.1.0
    DESCRIPTION "BlokkForge Core Library"
    LANGUAGES CXX
)

# Use C++20
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Header-only library
add_library(BFCL INTERFACE)

# Allow projects using BFCL to include headers
target_include_directories(BFCL INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/Structures/Safe
    ${CMAKE_CURRENT_SOURCE_DIR}/Structures/Unsafe
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# Require C++20 for projects using BFCL
target_compile_features(BFCL INTERFACE cxx_std_20)