cmake_minimum_required(VERSION 3.8)
project(P3)

set(CMAKE_CXX_STANDARD 11)

set(SOURCE_FILES main.c mem.h mem.c)
add_executable(P3 ${SOURCE_FILES})