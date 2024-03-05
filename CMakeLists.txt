cmake_minimum_required(VERSION 3.27)
project(rex)

set(CMAKE_C_STANDARD 99)
set(CMAKE_CXX_STANDARD 14)

add_executable(f64_test
        src/f64_test.cpp
        src/f64enc.h
        src/f64enc.c
        src/f64dec.h
        src/f64dec.c
)
