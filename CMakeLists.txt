cmake_minimum_required(VERSION 3.27)
project(rex C)

set(CMAKE_C_STANDARD 99)

add_executable(rex
		src/f64enc.h
		src/f64enc.c
)
