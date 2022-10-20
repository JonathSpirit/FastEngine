cmake_minimum_required(VERSION 3.10)
project(fge_install)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_executable(${PROJECT_NAME})

target_sources(${PROJECT_NAME} PRIVATE "main.cpp")

target_compile_options(${PROJECT_NAME} PRIVATE -Wpedantic -Wall -Wextra)
