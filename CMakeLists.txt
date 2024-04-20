cmake_minimum_required(VERSION 3.26)

project(rex LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)

add_executable(rex
    src/rex.cpp
    src/rex_generated.h
)

target_include_directories(rex PUBLIC
    lib/flatbuffers/include
)
