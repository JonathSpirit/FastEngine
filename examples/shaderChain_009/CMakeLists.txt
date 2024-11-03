cmake_minimum_required(VERSION 3.10)
project(example_shaderChain_009)

add_executable(${PROJECT_NAME} main.cpp)
add_dependencies(${PROJECT_NAME} FgeClientExeDeps)

target_link_libraries(${PROJECT_NAME} ${FGE_CLIENT_LIBS})

setMSVCDefaultWorkingDir(${PROJECT_NAME})

# Copy the shaders to the build directory
add_custom_target(${PROJECT_NAME}_CopyShaders)
add_custom_command(TARGET ${PROJECT_NAME}_CopyShaders PRE_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory_if_different
    ${CMAKE_CURRENT_SOURCE_DIR}/shaders
    ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/shaders
)

add_dependencies(${PROJECT_NAME} ${PROJECT_NAME}_CopyShaders)