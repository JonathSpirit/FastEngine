cmake_minimum_required(VERSION 3.10)
project(fge_install)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_executable(${PROJECT_NAME})

target_sources(${PROJECT_NAME} PRIVATE "main.cpp")

target_compile_options(${PROJECT_NAME} PRIVATE -Wpedantic -Wall -Wextra)

set(FGE_LIBS_DIR "../libs")

add_subdirectory(${FGE_LIBS_DIR}/re2 ${CMAKE_CURRENT_BINARY_DIR}/libs/re2) # re2
target_include_directories(${PROJECT_NAME} PRIVATE ${FGE_LIBS_DIR}/re2/re2)
add_dependencies(${PROJECT_NAME} re2)
target_link_libraries(${PROJECT_NAME} PRIVATE re2)
