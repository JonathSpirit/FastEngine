cmake_minimum_required(VERSION 3.10)
project(004_server)

add_executable(${PROJECT_NAME} main.cpp)
target_sources(${PROJECT_NAME} PRIVATE ../share/C_creature.cpp)
target_sources(${PROJECT_NAME} PRIVATE ../share/C_customObject.cpp)
target_sources(${PROJECT_NAME} PRIVATE ../share/C_drink.cpp)
target_sources(${PROJECT_NAME} PRIVATE ../share/C_food.cpp)
target_sources(${PROJECT_NAME} PRIVATE ../share/C_moveable.cpp)

target_include_directories(${PROJECT_NAME} PUBLIC ../share/includes)
target_compile_definitions(${PROJECT_NAME} PRIVATE FGE_DEF_SERVER)

add_dependencies(${PROJECT_NAME} FgeServerExeDeps)

target_link_libraries(${PROJECT_NAME} ${FGE_SERVER_LIBS})

setMSVCDefaultWorkingDir(${PROJECT_NAME})