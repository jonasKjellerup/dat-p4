cmake_minimum_required(VERSION 3.16.0)
list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)

project(p4 CXX)
set(CMAKE_CXX_STANDARD 20)

add_definitions(-DANTLR4CPP_STATIC)
include(ExternalAntlr4Cpp)
include_directories(includes ${ANTLR4_INCLUDE_DIRS})

set(ANTLR_EXECUTABLE ${PROJECT_SOURCE_DIR}/antlr.jar)
find_package(ANTLR REQUIRED)
find_package(Threads REQUIRED)



link_directories(${ANTLR4CPP_LIBS})

antlr_target(Eel ${PROJECT_SOURCE_DIR}/src/eel.g4 VISITOR
        PACKAGE eel
        OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/antlr-build)

include_directories(${ANTLR_Eel_OUTPUT_DIR})

add_compile_options(-Wall -Wextra -pedantic)

add_library(compiler
        ${ANTLR_Eel_CXX_OUTPUTS}
        ${ANTLR_Eel_CXX_OUTPUTS}
        src/parsing.cc
        src/symbol_table.cc
        src/symbols/type.cc
        src/error.cc
        src/symbols/event.cpp)
target_link_libraries(compiler antlr4_static)
target_link_libraries(compiler Threads::Threads)

add_executable(compiler_tests
        tests/entry.cc
        tests/test_antlr.cc
        tests/test_symbol_table.cc)
target_link_libraries(compiler_tests compiler)

add_executable(compiler_cli src/cli.cc)
target_link_libraries(compiler_cli compiler)
