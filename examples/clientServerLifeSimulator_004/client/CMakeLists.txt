cmake_minimum_required(VERSION 3.10)
project(004_client)

add_executable(${PROJECT_NAME} main.cpp)
target_sources(${PROJECT_NAME} PRIVATE ../share/C_creature.cpp)
target_sources(${PROJECT_NAME} PRIVATE ../share/C_customObject.cpp)
target_sources(${PROJECT_NAME} PRIVATE ../share/C_drink.cpp)
target_sources(${PROJECT_NAME} PRIVATE ../share/C_food.cpp)
target_sources(${PROJECT_NAME} PRIVATE ../share/C_moveable.cpp)

target_include_directories(${PROJECT_NAME} PUBLIC ../share/includes)

add_dependencies(${PROJECT_NAME} FgeClientExeDeps)

target_link_libraries(${PROJECT_NAME} ${FGE_CLIENT_LIBS})