cmake_minimum_required(VERSION 3.22.1)
project(p4 CXX)

set(CMAKE_CXX_STANDARD 20)

include_directories(includes)

add_compile_options(-Wall -Wextra -pedantic)

add_executable(compiler src/main.cc)
