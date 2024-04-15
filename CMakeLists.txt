cmake_minimum_required(VERSION 3.28)
project(rex LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)

add_library(rex STATIC
        src/rex.cpp
        src/rex_generated.h
)

target_include_directories(rex PRIVATE
        lib/flatbuffers/include
)
