cmake_minimum_required(VERSION 3.12)
project("my_ldd" LANGUAGES C CXX VERSION "0.1")
set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake")

add_executable(babylonfs
    ./src/main.cpp
)

target_compile_options(babylonfs PRIVATE -Wall -Wextra -pedantic)
target_compile_features(babylonfs PRIVATE cxx_std_20)

find_package(FUSE REQUIRED)
add_library(fuse INTERFACE)
target_compile_definitions(fuse INTERFACE ${FUSE_DEFINITIONS})
target_include_directories(fuse INTERFACE ${FUSE_INCLUDE_DIRS})
target_link_libraries(fuse INTERFACE ${FUSE_LIBRARIES})

target_link_libraries(babylonfs fuse)
