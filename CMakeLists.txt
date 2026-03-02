cmake_minimum_required(VERSION 3.25 FATAL_ERROR)
project(Impatiens VERSION 1.0.0 LANGUAGES C CXX)

option(IMPATIENS_BUILD_TESTS "Build tests" OFF)

add_library(Impatiens INTERFACE)

target_compile_features(Impatiens INTERFACE cxx_std_23)

target_include_directories(Impatiens INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

add_subdirectory(src)

if (IMPATIENS_BUILD_TESTS)
    add_subdirectory(test)
endif()
