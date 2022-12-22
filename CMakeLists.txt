cmake_minimum_required(VERSION 3.22.3)
project(job_shop)

set(CMAKE_CXX_STANDARD 20)

add_compile_options(-O3)

add_executable(job_shop main.cpp)
add_executable(test test.cpp)
add_executable(convert convert.cpp)