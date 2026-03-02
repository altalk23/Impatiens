cmake_minimum_required(VERSION 3.25 FATAL_ERROR)
project(ImpatiensSource VERSION 1.0.0 LANGUAGES C CXX)

add_library(ImpatiensSource STATIC
    Main.cpp
    Runtime.cpp
)

target_link_libraries(ImpatiensSource PUBLIC Impatiens)