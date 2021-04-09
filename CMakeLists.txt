cmake_minimum_required(VERSION 3.12)
project("my_ldd" LANGUAGES C CXX VERSION "0.1")
include(cmake/CPM.cmake)

add_executable(babylonfs
    ./src/main.cpp
)

target_compile_options(babylonfs PRIVATE -Wall -Wextra -pedantic)
target_compile_features(babylonfs PRIVATE cxx_std_20)

CPMAddPackage("gh:jarro2783/cxxopts#dd45a0801c99d62109aaa29f8c410ba8def2fbf2")

target_link_libraries(babylonfs cxxopts::cxxopts)
