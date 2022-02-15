cmake_minimum_required(VERSION 3.10)
project(FastEngine)

#Check for architecture
if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(FGE_ARCH 64)
else()
    set(FGE_ARCH 32)
endif()

if(CMAKE_BUILD_TYPE MATCHES Debug)
    set(FGE_LIB_NAME "${PROJECT_NAME}${FGE_ARCH}_d")
    set(FGE_SERVER_LIB_NAME "${PROJECT_NAME}Server${FGE_ARCH}_d")
    set(FGE_DEBUG ON)
else()
    set(FGE_LIB_NAME "${PROJECT_NAME}${FGE_ARCH}")
    set(FGE_SERVER_LIB_NAME "${PROJECT_NAME}Server${FGE_ARCH}")
    set(FGE_DEBUG OFF)
endif()

if(NOT WIN32)
    set(FGE_SFML_MAIN "")
else()
    set(FGE_SFML_MAIN "sfml-main")
endif()

#Getting all *.h/*.inl files
file(GLOB_RECURSE FGE_INCLUDES
        LIST_DIRECTORIES false
        RELATIVE ${PROJECT_SOURCE_DIR}
        includes/*.hpp includes/*.inl)

file(GLOB FGE_INCLUDES_EXTRA
        LIST_DIRECTORIES false
        RELATIVE ${PROJECT_SOURCE_DIR}
        includes/*.hpp includes/*.inl)

file(GLOB FGE_INCLUDES_PROJECT
        LIST_DIRECTORIES false
        RELATIVE ${PROJECT_SOURCE_DIR}
        includes/FastEngine/*.hpp includes/FastEngine/*.inl)

set(FGE_EXE_NAME ${PROJECT_NAME}_test)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

#Dependency
find_package(Git)
add_custom_target(version
        ${CMAKE_COMMAND} -D SRC=${CMAKE_SOURCE_DIR}/version/fastengine_version.hpp.in
        -D DST=${CMAKE_SOURCE_DIR}/includes/FastEngine/fastengine_version.hpp
        -D GIT_EXECUTABLE=${GIT_EXECUTABLE}
        -P ${CMAKE_SOURCE_DIR}/GenerateVersionHeader.cmake
        )

#CMAKE_BINARY_DIR
set(BUILD_SHARED_LIBS ON)
set(BUILD_STATIC_LIBS OFF)

set(SFML_BUILD_NETWORK OFF CACHE BOOL "" FORCE)
add_subdirectory("libs/SFML" EXCLUDE_FROM_ALL)

set(BUILD_SHARED_LIBS OFF)
set(BUILD_STATIC_LIBS ON)

set(LZ4_BUILD_CLI OFF CACHE BOOL "" FORCE)
add_subdirectory("libs/lz4/build/cmake" EXCLUDE_FROM_ALL)
set(FMT_TEST OFF CACHE BOOL "" FORCE)
add_subdirectory("libs/fmt" EXCLUDE_FROM_ALL)
set(RE2_BUILD_TESTING OFF CACHE BOOL "" FORCE)
set(BUILD_TESTING OFF CACHE BOOL "" FORCE)
add_subdirectory("libs/re2" EXCLUDE_FROM_ALL)
set(ENABLE_LIB_ONLY ON CACHE BOOL "" FORCE)
set(ENABLE_APP OFF CACHE BOOL "" FORCE)
set(ENABLE_STATIC_LIB ON CACHE BOOL "" FORCE)
add_subdirectory("libs/bzip2" EXCLUDE_FROM_ALL)

#Create the library
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_library(${FGE_LIB_NAME} SHARED)
add_library(${FGE_SERVER_LIB_NAME} SHARED)

#Create the test executable

add_executable(${FGE_EXE_NAME})
add_dependencies(${FGE_EXE_NAME} ${FGE_LIB_NAME})

#Dependencies
add_dependencies(${FGE_LIB_NAME} sfml-audio sfml-graphics sfml-system sfml-window)
add_dependencies(${FGE_LIB_NAME} version)
add_dependencies(${FGE_SERVER_LIB_NAME} sfml-graphics sfml-system sfml-window)
add_dependencies(${FGE_SERVER_LIB_NAME} version)
add_dependencies(${FGE_EXE_NAME} sfml-audio sfml-graphics ${FGE_SFML_MAIN} sfml-system sfml-window)

target_include_directories(${FGE_LIB_NAME} PUBLIC "libs/SFML/include")
target_include_directories(${FGE_SERVER_LIB_NAME} PUBLIC "libs/SFML/include")

add_dependencies(${FGE_LIB_NAME} lz4_static)
add_dependencies(${FGE_SERVER_LIB_NAME} lz4_static)
target_include_directories(${FGE_LIB_NAME} PUBLIC "libs/lz4/lib")
target_include_directories(${FGE_SERVER_LIB_NAME} PUBLIC "libs/lz4/lib")
add_dependencies(${FGE_LIB_NAME} fmt)
add_dependencies(${FGE_SERVER_LIB_NAME} fmt)
target_include_directories(${FGE_LIB_NAME} PUBLIC "libs/fmt/include")
target_include_directories(${FGE_SERVER_LIB_NAME} PUBLIC "libs/fmt/include")
add_dependencies(${FGE_LIB_NAME} re2)
add_dependencies(${FGE_SERVER_LIB_NAME} re2)
target_include_directories(${FGE_LIB_NAME} PUBLIC "libs/re2/re2")
target_include_directories(${FGE_SERVER_LIB_NAME} PUBLIC "libs/re2/re2")
add_dependencies(${FGE_LIB_NAME} bz2_static)
add_dependencies(${FGE_SERVER_LIB_NAME} bz2_static)
target_include_directories(${FGE_LIB_NAME} PUBLIC "libs/bzip2")
target_include_directories(${FGE_SERVER_LIB_NAME} PUBLIC "libs/bzip2")

target_include_directories(${FGE_LIB_NAME} PUBLIC "libs/strtk")
target_include_directories(${FGE_SERVER_LIB_NAME} PUBLIC "libs/strtk")

#Definition
target_compile_definitions(${FGE_LIB_NAME} PRIVATE _FGE_DEF_BUILDDLL)
target_compile_definitions(${FGE_SERVER_LIB_NAME} PRIVATE _FGE_DEF_BUILDDLL)
if(FGE_DEBUG)
    target_compile_definitions(${FGE_LIB_NAME} PRIVATE _FGE_DEF_DEBUG)
    target_compile_definitions(${FGE_SERVER_LIB_NAME} PRIVATE _FGE_DEF_DEBUG)
endif()

#Includes path
target_include_directories(${FGE_LIB_NAME} PUBLIC "includes")
target_include_directories(${FGE_LIB_NAME} PUBLIC "${PROJECT_BINARY_DIR}")

target_include_directories(${FGE_SERVER_LIB_NAME} PUBLIC "includes")
target_include_directories(${FGE_SERVER_LIB_NAME} PUBLIC "${PROJECT_BINARY_DIR}")

#Source files
target_sources(${FGE_SERVER_LIB_NAME} PRIVATE ${FGE_INCLUDES})

target_sources(${FGE_SERVER_LIB_NAME} PRIVATE "sources/C_animation.cpp")
target_sources(${FGE_SERVER_LIB_NAME} PRIVATE "sources/C_commandHandler.cpp")
target_sources(${FGE_SERVER_LIB_NAME} PRIVATE "sources/C_event.cpp")
target_sources(${FGE_SERVER_LIB_NAME} PRIVATE "sources/C_eventList.cpp")
target_sources(${FGE_SERVER_LIB_NAME} PRIVATE "sources/C_font.cpp")

target_sources(${FGE_SERVER_LIB_NAME} PRIVATE "sources/C_objAnim.cpp")
target_sources(${FGE_SERVER_LIB_NAME} PRIVATE "sources/C_objButton.cpp")
target_sources(${FGE_SERVER_LIB_NAME} PRIVATE "sources/C_object.cpp")
target_sources(${FGE_SERVER_LIB_NAME} PRIVATE "sources/C_objLight.cpp")
target_sources(${FGE_SERVER_LIB_NAME} PRIVATE "sources/C_objRenderMap.cpp")
target_sources(${FGE_SERVER_LIB_NAME} PRIVATE "sources/C_objSelectBox.cpp")
target_sources(${FGE_SERVER_LIB_NAME} PRIVATE "sources/C_objSprite.cpp")
target_sources(${FGE_SERVER_LIB_NAME} PRIVATE "sources/C_objSwitch.cpp")
target_sources(${FGE_SERVER_LIB_NAME} PRIVATE "sources/C_objText.cpp")
target_sources(${FGE_SERVER_LIB_NAME} PRIVATE "sources/C_objTextinputbox.cpp")

target_sources(${FGE_SERVER_LIB_NAME} PRIVATE "sources/C_random.cpp")

target_sources(${FGE_SERVER_LIB_NAME} PRIVATE "sources/C_client.cpp")
target_sources(${FGE_SERVER_LIB_NAME} PRIVATE "sources/C_clientList.cpp")
target_sources(${FGE_SERVER_LIB_NAME} PRIVATE "sources/C_ipAddress.cpp")
target_sources(${FGE_SERVER_LIB_NAME} PRIVATE "sources/C_networkType.cpp")
target_sources(${FGE_SERVER_LIB_NAME} PRIVATE "sources/C_packet.cpp")
target_sources(${FGE_SERVER_LIB_NAME} PRIVATE "sources/C_packetBZ2.cpp")
target_sources(${FGE_SERVER_LIB_NAME} PRIVATE "sources/C_packetLZ4.cpp")
target_sources(${FGE_SERVER_LIB_NAME} PRIVATE "sources/C_server.cpp")
target_sources(${FGE_SERVER_LIB_NAME} PRIVATE "sources/C_socket.cpp")

target_sources(${FGE_SERVER_LIB_NAME} PRIVATE "sources/C_scene.cpp")
target_sources(${FGE_SERVER_LIB_NAME} PRIVATE "sources/C_subscription.cpp")
target_sources(${FGE_SERVER_LIB_NAME} PRIVATE "sources/C_tagList.cpp")
target_sources(${FGE_SERVER_LIB_NAME} PRIVATE "sources/C_texture.cpp")
target_sources(${FGE_SERVER_LIB_NAME} PRIVATE "sources/C_timer.cpp")

target_sources(${FGE_SERVER_LIB_NAME} PRIVATE "sources/extra_function.cpp")
target_sources(${FGE_SERVER_LIB_NAME} PRIVATE "sources/extra_string.cpp")

target_sources(${FGE_SERVER_LIB_NAME} PRIVATE "sources/fge_drawing.cpp")
target_sources(${FGE_SERVER_LIB_NAME} PRIVATE "sources/fge_endian.cpp")

target_sources(${FGE_SERVER_LIB_NAME} PRIVATE "sources/anim_manager.cpp")
target_sources(${FGE_SERVER_LIB_NAME} PRIVATE "sources/crash_manager.cpp")
target_sources(${FGE_SERVER_LIB_NAME} PRIVATE "sources/font_manager.cpp")
target_sources(${FGE_SERVER_LIB_NAME} PRIVATE "sources/log_manager.cpp")
target_sources(${FGE_SERVER_LIB_NAME} PRIVATE "sources/network_manager.cpp")
target_sources(${FGE_SERVER_LIB_NAME} PRIVATE "sources/path_manager.cpp")
target_sources(${FGE_SERVER_LIB_NAME} PRIVATE "sources/reg_manager.cpp")
target_sources(${FGE_SERVER_LIB_NAME} PRIVATE "sources/screen_manager.cpp")
target_sources(${FGE_SERVER_LIB_NAME} PRIVATE "sources/texture_manager.cpp")
target_sources(${FGE_SERVER_LIB_NAME} PRIVATE "sources/timer_manager.cpp")

#######################

target_sources(${FGE_LIB_NAME} PRIVATE ${FGE_INCLUDES})

target_sources(${FGE_LIB_NAME} PRIVATE "sources/C_animation.cpp")
target_sources(${FGE_LIB_NAME} PRIVATE "sources/C_commandHandler.cpp")
target_sources(${FGE_LIB_NAME} PRIVATE "sources/C_event.cpp")
target_sources(${FGE_LIB_NAME} PRIVATE "sources/C_eventList.cpp")
target_sources(${FGE_LIB_NAME} PRIVATE "sources/C_font.cpp")

target_sources(${FGE_LIB_NAME} PRIVATE "sources/C_objAnim.cpp")
target_sources(${FGE_LIB_NAME} PRIVATE "sources/C_objButton.cpp")
target_sources(${FGE_LIB_NAME} PRIVATE "sources/C_object.cpp")
target_sources(${FGE_LIB_NAME} PRIVATE "sources/C_objLight.cpp")
target_sources(${FGE_LIB_NAME} PRIVATE "sources/C_objRenderMap.cpp")
target_sources(${FGE_LIB_NAME} PRIVATE "sources/C_objSelectBox.cpp")
target_sources(${FGE_LIB_NAME} PRIVATE "sources/C_objSprite.cpp")
target_sources(${FGE_LIB_NAME} PRIVATE "sources/C_objSwitch.cpp")
target_sources(${FGE_LIB_NAME} PRIVATE "sources/C_objText.cpp")
target_sources(${FGE_LIB_NAME} PRIVATE "sources/C_objTextinputbox.cpp")

target_sources(${FGE_LIB_NAME} PRIVATE "sources/C_random.cpp")

target_sources(${FGE_LIB_NAME} PRIVATE "sources/C_client.cpp")
target_sources(${FGE_LIB_NAME} PRIVATE "sources/C_clientList.cpp")
target_sources(${FGE_LIB_NAME} PRIVATE "sources/C_ipAddress.cpp")
target_sources(${FGE_LIB_NAME} PRIVATE "sources/C_networkType.cpp")
target_sources(${FGE_LIB_NAME} PRIVATE "sources/C_packet.cpp")
target_sources(${FGE_LIB_NAME} PRIVATE "sources/C_packetBZ2.cpp")
target_sources(${FGE_LIB_NAME} PRIVATE "sources/C_packetLZ4.cpp")
target_sources(${FGE_LIB_NAME} PRIVATE "sources/C_server.cpp")
target_sources(${FGE_LIB_NAME} PRIVATE "sources/C_socket.cpp")

target_sources(${FGE_LIB_NAME} PRIVATE "sources/C_scene.cpp")
target_sources(${FGE_LIB_NAME} PRIVATE "sources/C_soundBuffer.cpp")
target_sources(${FGE_LIB_NAME} PRIVATE "sources/C_subscription.cpp")
target_sources(${FGE_LIB_NAME} PRIVATE "sources/C_tagList.cpp")
target_sources(${FGE_LIB_NAME} PRIVATE "sources/C_texture.cpp")
target_sources(${FGE_LIB_NAME} PRIVATE "sources/C_timer.cpp")

target_sources(${FGE_LIB_NAME} PRIVATE "sources/extra_function.cpp")
target_sources(${FGE_LIB_NAME} PRIVATE "sources/extra_string.cpp")

target_sources(${FGE_LIB_NAME} PRIVATE "sources/fge_drawing.cpp")
target_sources(${FGE_LIB_NAME} PRIVATE "sources/fge_endian.cpp")

target_sources(${FGE_LIB_NAME} PRIVATE "sources/anim_manager.cpp")
target_sources(${FGE_LIB_NAME} PRIVATE "sources/audio_manager.cpp")
target_sources(${FGE_LIB_NAME} PRIVATE "sources/crash_manager.cpp")
target_sources(${FGE_LIB_NAME} PRIVATE "sources/font_manager.cpp")
target_sources(${FGE_LIB_NAME} PRIVATE "sources/log_manager.cpp")
target_sources(${FGE_LIB_NAME} PRIVATE "sources/network_manager.cpp")
target_sources(${FGE_LIB_NAME} PRIVATE "sources/path_manager.cpp")
target_sources(${FGE_LIB_NAME} PRIVATE "sources/reg_manager.cpp")
target_sources(${FGE_LIB_NAME} PRIVATE "sources/screen_manager.cpp")
target_sources(${FGE_LIB_NAME} PRIVATE "sources/texture_manager.cpp")
target_sources(${FGE_LIB_NAME} PRIVATE "sources/timer_manager.cpp")

target_sources(${FGE_EXE_NAME} PRIVATE "sources/main_debug.cpp")

#Linking library
if(WIN32)
    target_link_libraries(${FGE_LIB_NAME} user32 ws2_32)
    target_link_libraries(${FGE_SERVER_LIB_NAME} user32 ws2_32)
endif()

target_link_libraries(${FGE_LIB_NAME} bz2_static)
target_link_libraries(${FGE_LIB_NAME} lz4_static)
target_link_libraries(${FGE_LIB_NAME} fmt)
target_link_libraries(${FGE_LIB_NAME} sfml-audio sfml-graphics sfml-system sfml-window)
target_link_libraries(${FGE_LIB_NAME} re2)

target_link_libraries(${FGE_SERVER_LIB_NAME} bz2_static)
target_link_libraries(${FGE_SERVER_LIB_NAME} lz4_static)
target_link_libraries(${FGE_SERVER_LIB_NAME} fmt)
target_link_libraries(${FGE_SERVER_LIB_NAME} sfml-graphics sfml-system sfml-window)
target_link_libraries(${FGE_SERVER_LIB_NAME} re2)

target_link_libraries(${FGE_EXE_NAME} sfml-audio sfml-graphics ${FGE_SFML_MAIN} sfml-system sfml-window ${FGE_LIB_NAME})

#Install
install(TARGETS ${FGE_LIB_NAME})
install(TARGETS ${FGE_SERVER_LIB_NAME})
install(FILES ${FGE_INCLUDES_EXTRA}
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
install(FILES ${FGE_INCLUDES_PROJECT}
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/FastEngine)