cmake_minimum_required(VERSION 3.10)
project(004_client)

add_executable(${PROJECT_NAME} main.cpp)
target_sources(${PROJECT_NAME} PRIVATE ../share/C_creature.cpp)
target_sources(${PROJECT_NAME} PRIVATE ../share/C_customObject.cpp)
target_sources(${PROJECT_NAME} PRIVATE ../share/C_drink.cpp)
target_sources(${PROJECT_NAME} PRIVATE ../share/C_food.cpp)
target_sources(${PROJECT_NAME} PRIVATE ../share/C_moveable.cpp)

target_include_directories(${PROJECT_NAME} PUBLIC ../share/includes)

if(WIN32)
    target_link_libraries(${PROJECT_NAME} sfml-audio sfml-graphics ${FGE_SFML_MAIN} sfml-system sfml-window ${FGE_LIB_NAME})
elseif(APPLE)
    target_link_libraries(${PROJECT_NAME} sfml-audio sfml-graphics ${FGE_SFML_MAIN} sfml-system sfml-window ${FGE_LIB_NAME})
else()
    target_link_libraries(${PROJECT_NAME} sfml-audio sfml-graphics ${FGE_SFML_MAIN} sfml-system sfml-window X11 ${FGE_LIB_NAME})
endif()