cmake_minimum_required(VERSION 3.12)
project("babylonfs" LANGUAGES C CXX VERSION "0.1")
set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake")

add_executable(babylonfs
        src/main.cpp
        src/babylonfs.cpp
        src/logic.cpp
        src/util.cpp
)

add_executable(test
        src/babylonfs.cpp
        src/logic.cpp
        src/util.cpp
        test/tests.cpp
)

target_compile_options(babylonfs PRIVATE -Wall -Wextra -pedantic)
target_compile_features(babylonfs PRIVATE cxx_std_20)

find_package(FUSE REQUIRED)
add_library(fuse INTERFACE)
target_compile_definitions(fuse INTERFACE ${FUSE_DEFINITIONS})
target_include_directories(fuse INTERFACE ${FUSE_INCLUDE_DIRS})
target_link_libraries(fuse INTERFACE ${FUSE_LIBRARIES})

target_link_libraries(babylonfs fuse)
target_compile_definitions(babylonfs PRIVATE FUSE_USE_VERSION=26)

target_compile_features(test PRIVATE cxx_std_20)
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

target_link_libraries(test fuse Threads::Threads)
target_compile_definitions(test PRIVATE FUSE_USE_VERSION=26)
