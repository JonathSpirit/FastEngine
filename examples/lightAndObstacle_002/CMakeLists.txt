cmake_minimum_required(VERSION 3.10)
project(example_lightAndObstacle_002)

add_executable(${PROJECT_NAME} main.cpp)
add_dependencies(${PROJECT_NAME} FgeClientExeDeps)

target_link_libraries(${PROJECT_NAME} ${FGE_CLIENT_LIBS})

setMSVCDefaultWorkingDir(${PROJECT_NAME})