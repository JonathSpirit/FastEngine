cmake_minimum_required(VERSION 3.10)
project(deepText_005)

add_executable(${PROJECT_NAME} main.cpp)

if(WIN32)
    target_link_libraries(${PROJECT_NAME} sfml-audio sfml-graphics ${FGE_SFML_MAIN} sfml-system sfml-window ${FGE_LIB_NAME})
elseif(APPLE)
    target_link_libraries(${PROJECT_NAME} sfml-audio sfml-graphics ${FGE_SFML_MAIN} sfml-system sfml-window ${FGE_LIB_NAME})
else()
    target_link_libraries(${PROJECT_NAME} sfml-audio sfml-graphics ${FGE_SFML_MAIN} sfml-system sfml-window X11 ${FGE_LIB_NAME})
endif()
