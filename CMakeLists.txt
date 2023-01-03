cmake_minimum_required(VERSION 3.10)
project(FastEngine)

include(ExternalProject)

#Flags
if (NOT CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    add_compile_options(-Wno-error=stringop-overflow=) #Fix compile error with minimp3
endif()

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

#Get the GIT version target
find_package(Git)

#Get FastEngine version
set(SRC ${CMAKE_SOURCE_DIR}/version/fastengine_version.hpp.in)
set(DST ${CMAKE_SOURCE_DIR}/includes/FastEngine/fastengine_version.hpp)
include(${CMAKE_SOURCE_DIR}/GenerateVersionHeader.cmake)

#Options
if (CMAKE_BUILD_TYPE MATCHES "^[Rr]elease")
    option(FGE_BUILD_DOC "Build documentation" ON)
else()
    option(FGE_BUILD_DOC "Build documentation" OFF)
endif()

option(FGE_BUILD_EXAMPLES "Build examples" ON)
option(FGE_BUILD_TESTS "Build tests" ON)

#Check if Doxygen is installed
if (FGE_BUILD_DOC)
    find_package(Doxygen)
    if (DOXYGEN_FOUND)
        # set input and output files
        set(DOXYGEN_IN ${CMAKE_CURRENT_SOURCE_DIR}/docs/Doxyfile.in)
        set(DOXYGEN_OUT ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)

        # request to configure the file
        configure_file(${DOXYGEN_IN} ${DOXYGEN_OUT} @ONLY)
        message("Doxygen build started")

        # note the option ALL which allows to build the docs together with the application
        add_custom_target( doc_doxygen ALL
                COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYGEN_OUT}
                WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
                COMMENT "Generating API documentation with Doxygen"
                VERBATIM )

    else (DOXYGEN_FOUND)
        message("Doxygen need to be installed to generate the doxygen documentation")
    endif (DOXYGEN_FOUND)
endif()

#Add all subdirectories

#Make sure we are building shared libs for libraries that need to be shared
set(BUILD_SHARED_LIBS ON CACHE BOOL "" FORCE) #BEGIN BUILD_SHARED_LIBS
set(BUILD_STATIC_LIBS OFF CACHE BOOL "" FORCE)

if(WIN32)
    ExternalProject_Add(OpenAL_extern #OpenAL-soft
            URL "${PROJECT_SOURCE_DIR}/libs/openal-soft"
            PREFIX ${CMAKE_CURRENT_BINARY_DIR}/OpenAL_extern
            CMAKE_ARGS -DALSOFT_UTILS=OFF -DALSOFT_EXAMPLES=OFF -DALSOFT_RTKIT=OFF -DCMAKE_BUILD_TYPE=Release
            INSTALL_COMMAND ""
            TEST_COMMAND ""
            DOWNLOAD_COMMAND ""
            SOURCE_DIR ${PROJECT_SOURCE_DIR}/libs/openal-soft
            EXCLUDE_FROM_ALL
            )

    set(OPENAL_INCLUDE_DIR "${PROJECT_SOURCE_DIR}/libs/openal-soft/include/AL" CACHE PATH "" FORCE)
    set(OPENAL_LIBRARY "${CMAKE_CURRENT_BINARY_DIR}/OpenAL_extern/src/OpenAL_extern-build/libOpenAL32.dll.a" CACHE FILEPATH "" FORCE)

    # In order to set IMPORTED_LOCATION, SFML check if the filepath exist.
    # But if it's the first time that the user configure with cmake, the file will not be created yet.
    # SFML see that and try to populate IMPORTED_LIBNAME that doesn't accept filepath anymore.
    # In order to fix that, we create a dummy but valid file at the required path.
    if (NOT EXISTS ${OPENAL_LIBRARY})
        file(TOUCH ${OPENAL_LIBRARY})
    endif()

    ExternalProject_Add(freetype_extern #freetype
            URL "${PROJECT_SOURCE_DIR}/libs/freetype"
            PREFIX ${CMAKE_CURRENT_BINARY_DIR}/freetype_extern
            CMAKE_ARGS -DFT_DISABLE_HARFBUZZ=ON -DFT_DISABLE_ZLIB=ON -DFT_DISABLE_PNG=ON -DFT_DISABLE_BROTLI=ON -DFT_DISABLE_BZIP2=ON -DCMAKE_BUILD_TYPE=Release
            INSTALL_COMMAND ""
            TEST_COMMAND ""
            DOWNLOAD_COMMAND ""
            SOURCE_DIR ${PROJECT_SOURCE_DIR}/libs/freetype
            EXCLUDE_FROM_ALL
            )

    set(FREETYPE_INCLUDE_DIR "${PROJECT_SOURCE_DIR}/libs/freetype/include" CACHE PATH "" FORCE)
    set(FREETYPE_LIBRARY "${CMAKE_CURRENT_BINARY_DIR}/freetype_extern/src/freetype_extern-build/libfreetype.a" CACHE FILEPATH "" FORCE)

    if (NOT EXISTS ${FREETYPE_LIBRARY})
        file(TOUCH ${FREETYPE_LIBRARY})
    endif()

else()
    find_library(FREETYPE NAMES freetype REQUIRED)
    set(FREETYPE_INCLUDE_DIR ${FREETYPE_INCLUDE_DIR_freetype2})
endif()

set(SFML_BUILD_NETWORK OFF CACHE BOOL "" FORCE) #SFML
add_subdirectory("libs/SFML" EXCLUDE_FROM_ALL)

add_subdirectory(libs/SDL EXCLUDE_FROM_ALL)
set(SDL2IMAGE_INSTALL OFF CACHE BOOL "" FORCE)
add_subdirectory(libs/SDL_image EXCLUDE_FROM_ALL)

set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
set(BUILD_STATIC_LIBS ON CACHE BOOL "" FORCE) #END BUILD_SHARED_LIBS

set(LZ4_BUILD_CLI OFF CACHE BOOL "" FORCE) #lz4
add_subdirectory("libs/lz4/build/cmake" EXCLUDE_FROM_ALL)

set(FMT_TEST OFF CACHE BOOL "" FORCE) #fmt
add_subdirectory("libs/fmt" EXCLUDE_FROM_ALL)

set(RE2_BUILD_TESTING OFF CACHE BOOL "" FORCE) #re2
set(BUILD_TESTING OFF CACHE BOOL "" FORCE)
add_subdirectory("libs/re2" EXCLUDE_FROM_ALL)

set(ENABLE_LIB_ONLY ON CACHE BOOL "" FORCE) #bzip2
set(ENABLE_APP OFF CACHE BOOL "" FORCE)
set(ENABLE_STATIC_LIB ON CACHE BOOL "" FORCE)
add_subdirectory("libs/bzip2" EXCLUDE_FROM_ALL)

#Create the library
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_library(${FGE_LIB_NAME} SHARED)
add_library(${FGE_SERVER_LIB_NAME} SHARED)

#Dependencies

#Checking for Vulkan
find_package(Vulkan MODULE REQUIRED)
message(${Vulkan_GLSLC_EXECUTABLE})

set(VOLK_PATH ${CMAKE_CURRENT_SOURCE_DIR}/libs/volk)

if (WIN32)
    add_compile_definitions(VK_USE_PLATFORM_WIN32_KHR)
endif()

if(WIN32)
    add_dependencies(sfml-audio OpenAL_extern)
    add_dependencies(sfml-graphics freetype_extern)
    add_dependencies(${FGE_SERVER_LIB_NAME} freetype_extern)
    add_dependencies(${FGE_LIB_NAME} freetype_extern)
endif()

add_dependencies(${FGE_LIB_NAME} SDL2 SDL2_image) #SDL
target_include_directories(${FGE_LIB_NAME} PUBLIC ${VOLK_PATH}) #Volk
target_include_directories(${FGE_LIB_NAME} PUBLIC libs/SDL/include)
target_include_directories(${FGE_LIB_NAME} PUBLIC libs/SDL_image)
target_include_directories(${FGE_LIB_NAME} PUBLIC libs/glm) #glm
include_directories(${Vulkan_INCLUDE_DIRS})

add_dependencies(${FGE_LIB_NAME} sfml-audio sfml-graphics sfml-system sfml-window) #SFML
add_dependencies(${FGE_SERVER_LIB_NAME} sfml-system)
target_include_directories(${FGE_LIB_NAME} PUBLIC "libs/SFML/include")
target_include_directories(${FGE_SERVER_LIB_NAME} PUBLIC "libs/SFML/include")

add_dependencies(${FGE_LIB_NAME} lz4_static) #Lz4
add_dependencies(${FGE_SERVER_LIB_NAME} lz4_static)
target_include_directories(${FGE_LIB_NAME} PUBLIC "libs/lz4/lib")
target_include_directories(${FGE_SERVER_LIB_NAME} PUBLIC "libs/lz4/lib")

add_dependencies(${FGE_LIB_NAME} fmt) #fmt
add_dependencies(${FGE_SERVER_LIB_NAME} fmt)
target_include_directories(${FGE_LIB_NAME} PUBLIC "libs/fmt/include")
target_include_directories(${FGE_SERVER_LIB_NAME} PUBLIC "libs/fmt/include")

add_dependencies(${FGE_LIB_NAME} re2) #re2
add_dependencies(${FGE_SERVER_LIB_NAME} re2)
target_include_directories(${FGE_LIB_NAME} PUBLIC "libs/re2/re2")
target_include_directories(${FGE_SERVER_LIB_NAME} PUBLIC "libs/re2/re2")

add_dependencies(${FGE_LIB_NAME} bz2_static) #bz2
add_dependencies(${FGE_SERVER_LIB_NAME} bz2_static)
target_include_directories(${FGE_LIB_NAME} PUBLIC "libs/bzip2")
target_include_directories(${FGE_SERVER_LIB_NAME} PUBLIC "libs/bzip2")

target_include_directories(${FGE_LIB_NAME} PUBLIC "libs/strtk") #strtk
target_include_directories(${FGE_SERVER_LIB_NAME} PUBLIC "libs/strtk")

target_include_directories(${FGE_LIB_NAME} PRIVATE ${FREETYPE_INCLUDE_DIR}) #FreeType
target_include_directories(${FGE_SERVER_LIB_NAME} PRIVATE ${FREETYPE_INCLUDE_DIR})

#Compile options
target_compile_options(${FGE_LIB_NAME} PRIVATE -Wpedantic -Wall -Wextra)
target_compile_options(${FGE_SERVER_LIB_NAME} PRIVATE -Wpedantic -Wall -Wextra)

#Definition
target_compile_definitions(${FGE_LIB_NAME} PRIVATE _FGE_DEF_BUILDDLL)
target_compile_definitions(${FGE_SERVER_LIB_NAME} PRIVATE _FGE_DEF_BUILDDLL)
if(FGE_DEBUG)
    target_compile_definitions(${FGE_LIB_NAME} PRIVATE FGE_DEF_DEBUG)
    target_compile_definitions(${FGE_SERVER_LIB_NAME} PRIVATE FGE_DEF_DEBUG)
endif()
target_compile_definitions(${FGE_SERVER_LIB_NAME} PRIVATE FGE_DEF_SERVER)

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
target_sources(${FGE_SERVER_LIB_NAME} PRIVATE "sources/C_property.cpp")

target_sources(${FGE_SERVER_LIB_NAME} PRIVATE "sources/object/C_objAnim.cpp")
target_sources(${FGE_SERVER_LIB_NAME} PRIVATE "sources/object/C_objButton.cpp")
target_sources(${FGE_SERVER_LIB_NAME} PRIVATE "sources/object/C_object.cpp")
target_sources(${FGE_SERVER_LIB_NAME} PRIVATE "sources/object/C_objectAnchor.cpp")
target_sources(${FGE_SERVER_LIB_NAME} PRIVATE "sources/object/C_objLight.cpp")
#target_sources(${FGE_SERVER_LIB_NAME} PRIVATE "sources/object/C_objRenderMap.cpp")
target_sources(${FGE_SERVER_LIB_NAME} PRIVATE "sources/object/C_objSelectBox.cpp")
target_sources(${FGE_SERVER_LIB_NAME} PRIVATE "sources/object/C_objSprite.cpp")
target_sources(${FGE_SERVER_LIB_NAME} PRIVATE "sources/object/C_objSwitch.cpp")
target_sources(${FGE_SERVER_LIB_NAME} PRIVATE "sources/object/C_objText.cpp")
target_sources(${FGE_SERVER_LIB_NAME} PRIVATE "sources/object/C_objTextinputbox.cpp")
target_sources(${FGE_SERVER_LIB_NAME} PRIVATE "sources/object/C_objTilemap.cpp")
target_sources(${FGE_SERVER_LIB_NAME} PRIVATE "sources/object/C_objWindow.cpp")
target_sources(${FGE_SERVER_LIB_NAME} PRIVATE "sources/object/C_objTextList.cpp")
target_sources(${FGE_SERVER_LIB_NAME} PRIVATE "sources/object/C_objSlider.cpp")

target_sources(${FGE_SERVER_LIB_NAME} PRIVATE "sources/C_guiElement.cpp")

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

target_sources(${FGE_SERVER_LIB_NAME} PRIVATE "sources/object/C_childObjectsAccessor.cpp")
target_sources(${FGE_SERVER_LIB_NAME} PRIVATE "sources/C_scene.cpp")
target_sources(${FGE_SERVER_LIB_NAME} PRIVATE "sources/C_subscription.cpp")
target_sources(${FGE_SERVER_LIB_NAME} PRIVATE "sources/C_tagList.cpp")
target_sources(${FGE_SERVER_LIB_NAME} PRIVATE "sources/C_texture.cpp")
target_sources(${FGE_SERVER_LIB_NAME} PRIVATE "sources/C_tileset.cpp")
target_sources(${FGE_SERVER_LIB_NAME} PRIVATE "sources/C_tilelayer.cpp")
target_sources(${FGE_SERVER_LIB_NAME} PRIVATE "sources/C_timer.cpp")

target_sources(${FGE_SERVER_LIB_NAME} PRIVATE "sources/extra/extra_function.cpp")
target_sources(${FGE_SERVER_LIB_NAME} PRIVATE "sources/extra/extra_string.cpp")
target_sources(${FGE_SERVER_LIB_NAME} PRIVATE "sources/extra/extra_pathFinding.cpp")

#target_sources(${FGE_SERVER_LIB_NAME} PRIVATE "sources/fge_drawing.cpp")
target_sources(${FGE_SERVER_LIB_NAME} PRIVATE "sources/fge_endian.cpp")

target_sources(${FGE_SERVER_LIB_NAME} PRIVATE "sources/manager/anim_manager.cpp")
#target_sources(${FGE_SERVER_LIB_NAME} PRIVATE "sources/manager/crash_manager.cpp")
target_sources(${FGE_SERVER_LIB_NAME} PRIVATE "sources/manager/font_manager.cpp")
target_sources(${FGE_SERVER_LIB_NAME} PRIVATE "sources/manager/log_manager.cpp")
target_sources(${FGE_SERVER_LIB_NAME} PRIVATE "sources/manager/network_manager.cpp")
target_sources(${FGE_SERVER_LIB_NAME} PRIVATE "sources/manager/path_manager.cpp")
target_sources(${FGE_SERVER_LIB_NAME} PRIVATE "sources/manager/reg_manager.cpp")
#target_sources(${FGE_SERVER_LIB_NAME} PRIVATE "sources/manager/screen_manager.cpp")
target_sources(${FGE_SERVER_LIB_NAME} PRIVATE "sources/manager/texture_manager.cpp")
target_sources(${FGE_SERVER_LIB_NAME} PRIVATE "sources/manager/timer_manager.cpp")

if(NOT WIN32)
    set(FREETYPE_INCLUDE_DIR ${FREETYPE_INCLUDE_DIR_freetype2})
endif()
set(FGE_SFML_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/libs/SFML)

target_compile_definitions(${FGE_SERVER_LIB_NAME} PRIVATE SFML_GRAPHICS_EXPORTS SFML_WINDOW_EXPORTS)
target_include_directories(${FGE_SERVER_LIB_NAME} PRIVATE includes/private/server)
target_include_directories(${FGE_SERVER_LIB_NAME} PRIVATE ${FGE_SFML_DIRECTORY}/extlibs/headers/stb_image)
target_include_directories(${FGE_SERVER_LIB_NAME} PRIVATE ${FGE_SFML_DIRECTORY}/src)

target_sources(${FGE_SERVER_LIB_NAME} PRIVATE "sources/private/server/RenderTarget.cpp")
target_sources(${FGE_SERVER_LIB_NAME} PRIVATE "sources/private/server/GlResource.cpp")
target_sources(${FGE_SERVER_LIB_NAME} PRIVATE "sources/private/server/Texture.cpp")

target_sources(${FGE_SERVER_LIB_NAME} PRIVATE "${FGE_SFML_DIRECTORY}/src/SFML/Graphics/Color.cpp")
target_sources(${FGE_SERVER_LIB_NAME} PRIVATE "${FGE_SFML_DIRECTORY}/src/SFML/Graphics/ImageLoader.cpp")
target_sources(${FGE_SERVER_LIB_NAME} PRIVATE "${FGE_SFML_DIRECTORY}/src/SFML/Graphics/Image.cpp")
target_sources(${FGE_SERVER_LIB_NAME} PRIVATE "${FGE_SFML_DIRECTORY}/src/SFML/Graphics/Vertex.cpp")
target_sources(${FGE_SERVER_LIB_NAME} PRIVATE "${FGE_SFML_DIRECTORY}/src/SFML/Graphics/VertexArray.cpp")
target_sources(${FGE_SERVER_LIB_NAME} PRIVATE "${FGE_SFML_DIRECTORY}/src/SFML/Graphics/Transform.cpp")
target_sources(${FGE_SERVER_LIB_NAME} PRIVATE "${FGE_SFML_DIRECTORY}/src/SFML/Graphics/Transformable.cpp")
target_sources(${FGE_SERVER_LIB_NAME} PRIVATE "${FGE_SFML_DIRECTORY}/src/SFML/Graphics/BlendMode.cpp")
target_sources(${FGE_SERVER_LIB_NAME} PRIVATE "${FGE_SFML_DIRECTORY}/src/SFML/Graphics/Shape.cpp")
target_sources(${FGE_SERVER_LIB_NAME} PRIVATE "${FGE_SFML_DIRECTORY}/src/SFML/Graphics/RectangleShape.cpp")
target_sources(${FGE_SERVER_LIB_NAME} PRIVATE "${FGE_SFML_DIRECTORY}/src/SFML/Graphics/CircleShape.cpp")
target_sources(${FGE_SERVER_LIB_NAME} PRIVATE "${FGE_SFML_DIRECTORY}/src/SFML/Graphics/ConvexShape.cpp")
target_sources(${FGE_SERVER_LIB_NAME} PRIVATE "${FGE_SFML_DIRECTORY}/src/SFML/Graphics/View.cpp")
target_sources(${FGE_SERVER_LIB_NAME} PRIVATE "${FGE_SFML_DIRECTORY}/src/SFML/Graphics/RenderStates.cpp")
target_sources(${FGE_SERVER_LIB_NAME} PRIVATE "${FGE_SFML_DIRECTORY}/src/SFML/Graphics/Font.cpp")
target_sources(${FGE_SERVER_LIB_NAME} PRIVATE "${FGE_SFML_DIRECTORY}/src/SFML/Graphics/Text.cpp")

#######################

target_sources(${FGE_LIB_NAME} PRIVATE ${FGE_INCLUDES})

target_sources(${FGE_LIB_NAME} PRIVATE ${VOLK_PATH}/volk.c)
target_sources(${FGE_LIB_NAME} PRIVATE ${VOLK_PATH}/volk.h)

target_sources(${FGE_LIB_NAME} PRIVATE
        sources/vulkan/vulkanGlobal.cpp
        sources/vulkan/C_instance.cpp
        sources/vulkan/C_physicalDevice.cpp
        sources/vulkan/C_logicalDevice.cpp
        sources/vulkan/C_uniformBuffer.cpp
        sources/vulkan/C_vertexBuffer.cpp
        sources/vulkan/C_textureImage.cpp
        sources/vulkan/C_swapChain.cpp
        sources/vulkan/C_surface.cpp
        sources/vulkan/C_shader.cpp
        sources/vulkan/C_descriptorPool.cpp
        sources/vulkan/C_descriptorSet.cpp
        sources/vulkan/C_descriptorSetLayout.cpp
        sources/vulkan/C_context.cpp
        sources/vulkan/C_graphicPipeline.cpp
        sources/vulkan/C_blendMode.cpp
        sources/vulkan/C_viewport.cpp)

target_sources(${FGE_LIB_NAME} PRIVATE
        sources/graphic/C_surface.cpp
        sources/graphic/C_view.cpp
        sources/graphic/C_transformable.cpp
        sources/graphic/C_shape.cpp
        sources/graphic/C_rectangleShape.cpp
        sources/graphic/C_renderWindow.cpp
        sources/graphic/C_renderTexture.cpp
        sources/graphic/C_renderTarget.cpp
        sources/graphic/C_ftFont.cpp)

target_sources(${FGE_LIB_NAME} PRIVATE "sources/C_animation.cpp")
target_sources(${FGE_LIB_NAME} PRIVATE "sources/C_commandHandler.cpp")
target_sources(${FGE_LIB_NAME} PRIVATE "sources/C_event.cpp")
target_sources(${FGE_LIB_NAME} PRIVATE "sources/C_eventList.cpp")
target_sources(${FGE_LIB_NAME} PRIVATE "sources/C_font.cpp")
target_sources(${FGE_LIB_NAME} PRIVATE "sources/C_property.cpp")

#target_sources(${FGE_LIB_NAME} PRIVATE "sources/object/C_objAnim.cpp")
#target_sources(${FGE_LIB_NAME} PRIVATE "sources/object/C_objButton.cpp")
target_sources(${FGE_LIB_NAME} PRIVATE "sources/object/C_object.cpp")
target_sources(${FGE_LIB_NAME} PRIVATE "sources/object/C_objectAnchor.cpp")
#target_sources(${FGE_LIB_NAME} PRIVATE "sources/object/C_objLight.cpp")
#target_sources(${FGE_LIB_NAME} PRIVATE "sources/object/C_objRenderMap.cpp")
#target_sources(${FGE_LIB_NAME} PRIVATE "sources/object/C_objSelectBox.cpp")
target_sources(${FGE_LIB_NAME} PRIVATE "sources/object/C_objSprite.cpp")
#target_sources(${FGE_LIB_NAME} PRIVATE "sources/object/C_objSwitch.cpp")
target_sources(${FGE_LIB_NAME} PRIVATE "sources/object/C_objText.cpp")
#target_sources(${FGE_LIB_NAME} PRIVATE "sources/object/C_objTextinputbox.cpp")
#target_sources(${FGE_LIB_NAME} PRIVATE "sources/object/C_objTilemap.cpp")
#target_sources(${FGE_LIB_NAME} PRIVATE "sources/object/C_objWindow.cpp")
#target_sources(${FGE_LIB_NAME} PRIVATE "sources/object/C_objTextList.cpp")
target_sources(${FGE_LIB_NAME} PRIVATE "sources/object/C_objSlider.cpp")

target_sources(${FGE_LIB_NAME} PRIVATE "sources/C_guiElement.cpp")

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

target_sources(${FGE_LIB_NAME} PRIVATE "sources/object/C_childObjectsAccessor.cpp")
target_sources(${FGE_LIB_NAME} PRIVATE "sources/C_scene.cpp")
target_sources(${FGE_LIB_NAME} PRIVATE "sources/C_soundBuffer.cpp")
target_sources(${FGE_LIB_NAME} PRIVATE "sources/C_subscription.cpp")
target_sources(${FGE_LIB_NAME} PRIVATE "sources/C_tagList.cpp")
target_sources(${FGE_LIB_NAME} PRIVATE "sources/C_texture.cpp")
target_sources(${FGE_LIB_NAME} PRIVATE "sources/C_tileset.cpp")
target_sources(${FGE_LIB_NAME} PRIVATE "sources/C_tilelayer.cpp")
target_sources(${FGE_LIB_NAME} PRIVATE "sources/C_timer.cpp")

target_sources(${FGE_LIB_NAME} PRIVATE "sources/extra/extra_function.cpp")
target_sources(${FGE_LIB_NAME} PRIVATE "sources/extra/extra_string.cpp")
target_sources(${FGE_LIB_NAME} PRIVATE "sources/extra/extra_pathFinding.cpp")

target_sources(${FGE_LIB_NAME} PRIVATE "sources/fge_drawing.cpp")
target_sources(${FGE_LIB_NAME} PRIVATE "sources/fge_endian.cpp")

target_sources(${FGE_LIB_NAME} PRIVATE "sources/manager/anim_manager.cpp")
target_sources(${FGE_LIB_NAME} PRIVATE "sources/manager/audio_manager.cpp")
target_sources(${FGE_LIB_NAME} PRIVATE "sources/manager/crash_manager.cpp")
target_sources(${FGE_LIB_NAME} PRIVATE "sources/manager/font_manager.cpp")
target_sources(${FGE_LIB_NAME} PRIVATE "sources/manager/log_manager.cpp")
target_sources(${FGE_LIB_NAME} PRIVATE "sources/manager/network_manager.cpp")
target_sources(${FGE_LIB_NAME} PRIVATE "sources/manager/path_manager.cpp")
target_sources(${FGE_LIB_NAME} PRIVATE "sources/manager/reg_manager.cpp")
target_sources(${FGE_LIB_NAME} PRIVATE "sources/manager/screen_manager.cpp")
target_sources(${FGE_LIB_NAME} PRIVATE "sources/manager/texture_manager.cpp")
target_sources(${FGE_LIB_NAME} PRIVATE "sources/manager/shader_manager.cpp")
target_sources(${FGE_LIB_NAME} PRIVATE "sources/manager/timer_manager.cpp")

#Linking library
if(WIN32)
    target_link_libraries(${FGE_LIB_NAME} user32 ws2_32)
    target_link_libraries(${FGE_SERVER_LIB_NAME} user32 ws2_32)
endif()

target_link_libraries(${FGE_LIB_NAME} SDL2 SDL2_image)

target_link_libraries(${FGE_LIB_NAME} bz2_static)
target_link_libraries(${FGE_LIB_NAME} lz4_static)
target_link_libraries(${FGE_LIB_NAME} fmt)
target_link_libraries(${FGE_LIB_NAME} ${FREETYPE_LIBRARY} sfml-audio sfml-graphics sfml-system sfml-window)
target_link_libraries(${FGE_LIB_NAME} re2)

target_link_libraries(${FGE_SERVER_LIB_NAME} bz2_static)
target_link_libraries(${FGE_SERVER_LIB_NAME} lz4_static)
target_link_libraries(${FGE_SERVER_LIB_NAME} fmt)
target_link_libraries(${FGE_SERVER_LIB_NAME} ${FREETYPE_LIBRARY} sfml-system)
target_link_libraries(${FGE_SERVER_LIB_NAME} re2)

#Compile shaders
add_custom_target(CompileShaders)

add_custom_command(TARGET CompileShaders PRE_BUILD
        COMMAND ${Vulkan_GLSLC_EXECUTABLE} ARGS vertex.vert -o vertex.spv
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/resources/shaders)
add_custom_command(TARGET CompileShaders PRE_BUILD
        COMMAND ${Vulkan_GLSLC_EXECUTABLE} ARGS fragmentTexture.frag -o fragmentTexture.spv
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/resources/shaders)
add_custom_command(TARGET CompileShaders PRE_BUILD
        COMMAND ${Vulkan_GLSLC_EXECUTABLE} ARGS fragment.frag -o fragment.spv
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/resources/shaders)

add_dependencies(${FGE_LIB_NAME} CompileShaders)

#Install
install(TARGETS ${FGE_LIB_NAME})
install(TARGETS ${FGE_SERVER_LIB_NAME})
install(FILES ${FGE_INCLUDES_EXTRA}
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
install(FILES ${FGE_INCLUDES_PROJECT}
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/FastEngine)

#Custom target for executables that copy compiled DLLs and symlink the resources folder
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

add_custom_target(FgeClientExeDeps)
add_custom_target(FgeServerExeDeps)
add_custom_target(FgeExeDeps) #TODO REMOVE THAT
add_dependencies(FgeClientExeDeps ${FGE_LIB_NAME})
add_dependencies(FgeServerExeDeps ${FGE_SERVER_LIB_NAME})
add_dependencies(FgeExeDeps ${FGE_LIB_NAME} ${FGE_SERVER_LIB_NAME}) #TODO REMOVE THAT

if (WIN32)
    foreach (DEPENDENCY sfml-system SDL2 SDL2_image)
        add_custom_command(TARGET FgeServerExeDeps PRE_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                $<TARGET_FILE:${DEPENDENCY}>
                ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
    endforeach()

    foreach (DEPENDENCY sfml-audio sfml-graphics sfml-system sfml-window SDL2 SDL2_image)
        add_custom_command(TARGET FgeClientExeDeps PRE_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                $<TARGET_FILE:${DEPENDENCY}>
                ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
    endforeach()
    add_custom_command(TARGET FgeClientExeDeps PRE_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${CMAKE_CURRENT_BINARY_DIR}/OpenAL_extern/src/OpenAL_extern-build/OpenAL32.dll
            ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
endif()

add_custom_command(TARGET FgeClientExeDeps PRE_BUILD
        COMMAND ${CMAKE_COMMAND} -E create_symlink
        ${CMAKE_CURRENT_SOURCE_DIR}/resources
        ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/resources)

add_custom_command(TARGET FgeServerExeDeps PRE_BUILD
        COMMAND ${CMAKE_COMMAND} -E create_symlink
        ${CMAKE_CURRENT_SOURCE_DIR}/resources
        ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/resources)

#Set some variables that help exe linking
if(WIN32)
    set(FGE_CLIENT_LIBS SDL2main SDL2 SDL2_image sfml-system ${FGE_LIB_NAME})
elseif(APPLE)
    set(FGE_CLIENT_LIBS SDL2main SDL2 SDL2_image sfml-system ${FGE_LIB_NAME})
else()
    set(FGE_CLIENT_LIBS SDL2main SDL2 SDL2_image sfml-system X11 ${FGE_LIB_NAME})
endif()

#Tests
if (FGE_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

#Examples
if (FGE_BUILD_EXAMPLES)
    add_subdirectory(examples/tileMapAndPathfinding_001)
    add_subdirectory(examples/lightAndObstacle_002)
    add_subdirectory(examples/guiWindow_003)
    add_subdirectory(examples/clientServerLifeSimulator_004)
    add_subdirectory(examples/deepText_005)
endif()

add_executable(testVulkan mainVulkanTest.cpp) #TODO REMOVE THAT
target_link_libraries(testVulkan SDL2main SDL2 SDL2_image sfml-system ${FGE_LIB_NAME})
