cmake_minimum_required(VERSION 3.26)

project(rex LANGUAGES C)

set(CMAKE_C_STANDARD 99)

add_executable(rcc
        ./rcc.c
)

target_include_directories(rcc PUBLIC
        ../lib/flatcc/include
)
