cmake_minimum_required(VERSION 3.27)
project(rex LANGUAGES C CXX)

set(CMAKE_C_STANDARD 99)
set(CMAKE_CXX_STANDARD 14)

add_subdirectory(lib/Catch2)

add_executable(f64_test
        src/f64enc.h
        src/f64enc.c
        src/f64dec.h
        src/f64dec.c
        src/f64enc_test.cpp
        src/f64_test.cpp
)
target_link_libraries(f64_test PRIVATE Catch2::Catch2WithMain)

include(CTest)
include(Catch)
catch_discover_tests(f64_test)
