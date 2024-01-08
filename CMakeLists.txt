cmake_minimum_required(VERSION 3.10)
project(FastEngine)

#Dependencies
if (NOT FGE_DEPS_PATH)
    #Throw error message if FGE_DEPS_PATH is not set
    message(FATAL_ERROR "FGE_DEPS_PATH is not set, please set it to the path of the dependencies (.../libs) folder create by the cmake script in deps/CMakeLists.txt")
else()
    set(FGE_DEPS_FULL_PATH ${CMAKE_CURRENT_SOURCE_DIR}/${FGE_DEPS_PATH})

    #Verify that FGE_DEPS_PATH contain "Dependencies.cmake"
    if (NOT EXISTS ${FGE_DEPS_FULL_PATH})
        message(FATAL_ERROR "FGE_DEPS_PATH is not a valid directory")
    else()
        if (NOT EXISTS ${FGE_DEPS_FULL_PATH}/Dependencies.cmake)
            message(FATAL_ERROR "FGE_DEPS_PATH does not contain Dependencies.cmake")
        endif()

        #Include the dependencies file
        include(${FGE_DEPS_FULL_PATH}/Dependencies.cmake)
    endif()
endif()

#Add a fix for lz4 on apple
if (APPLE)
    set_target_properties(LZ4::lz4_static PROPERTIES
            INTERFACE_INCLUDE_DIRECTORIES "${FGE_DEPS_FULL_PATH}/lz4/include"
    )
endif()

#Check for architecture
if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(FGE_ARCH 64)
else()
    set(FGE_ARCH 32)
endif()

if(CMAKE_BUILD_TYPE MATCHES "^[Dd]ebug")
    set(FGE_LIB_NAME "${PROJECT_NAME}${FGE_ARCH}_d")
    set(FGE_SERVER_LIB_NAME "${PROJECT_NAME}Server${FGE_ARCH}_d")
    set(FGE_DEBUG ON)
    set(FGE_ENABLE_VALIDATION_LAYERS ON CACHE BOOL "enable/disable Vulkan validation layers")
else()
    set(FGE_LIB_NAME "${PROJECT_NAME}${FGE_ARCH}")
    set(FGE_SERVER_LIB_NAME "${PROJECT_NAME}Server${FGE_ARCH}")
    set(FGE_DEBUG OFF)
    set(FGE_ENABLE_VALIDATION_LAYERS OFF CACHE BOOL "enable/disable Vulkan validation layers")
endif()

set(FGE_ENABLE_SERVER_NETWORK_RANDOM_LOST OFF CACHE BOOL "enable/disable random packet lost for server")
set(FGE_ENABLE_CLIENT_NETWORK_RANDOM_LOST OFF CACHE BOOL "enable/disable random packet lost for client")

if(APPLE)
    add_compile_options(-Wno-nullability-extension)
endif()

if(NOT WIN32)
    set(FGE_SDL_MAIN "")
else()
    set(FGE_SDL_MAIN "SDL2::SDL2main")
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
set(SRC ${CMAKE_SOURCE_DIR}/version/fge_version.hpp.in)
set(DST ${CMAKE_SOURCE_DIR}/includes/FastEngine/fge_version.hpp)
include(${CMAKE_SOURCE_DIR}/GenerateVersionHeader.cmake)

#Options
if (NOT ${FGE_DEBUG})
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

#Create the library
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_library(${FGE_LIB_NAME} SHARED)
add_library(${FGE_SERVER_LIB_NAME} SHARED)

if(WIN32)
    add_compile_definitions(VK_USE_PLATFORM_WIN32_KHR)
elseif(APPLE)
    add_compile_definitions(VK_USE_PLATFORM_MACOS_MVK)
else()
    add_compile_definitions(VK_USE_PLATFORM_XLIB_KHR)
endif()

add_compile_definitions(VMA_VULKAN_VERSION=1001000) #VulkanMemoryAllocator
add_compile_definitions(VMA_STATIC_VULKAN_FUNCTIONS=0)
add_compile_definitions(VMA_DYNAMIC_VULKAN_FUNCTIONS=0)

get_filename_component(DEPS_strtk_HEADER_FOLDER ${DEPS_strtk_HEADER_FILE} DIRECTORY) #strtk
target_include_directories(${FGE_LIB_NAME} PUBLIC ${DEPS_strtk_HEADER_FOLDER})
target_include_directories(${FGE_SERVER_LIB_NAME} PUBLIC ${DEPS_strtk_HEADER_FOLDER})

#Compile options
if (MSVC)
    target_compile_options(${FGE_LIB_NAME} PUBLIC /W4 /wd4251 /wd4275 /wd26812)
    target_compile_options(${FGE_SERVER_LIB_NAME} PUBLIC /W4 /wd4251 /wd4275 /wd26812)
else()
    target_compile_options(${FGE_LIB_NAME} PUBLIC -Wpedantic -Wall -Wextra)
    target_compile_options(${FGE_SERVER_LIB_NAME} PUBLIC -Wpedantic -Wall -Wextra)
endif()

#Definition
target_compile_definitions(${FGE_LIB_NAME} PRIVATE _FGE_DEF_BUILDDLL)
target_compile_definitions(${FGE_SERVER_LIB_NAME} PRIVATE _FGE_DEF_BUILDDLL)

if (${FGE_DEBUG})
    target_compile_definitions(${FGE_LIB_NAME} PRIVATE FGE_DEF_DEBUG)
    target_compile_definitions(${FGE_SERVER_LIB_NAME} PRIVATE FGE_DEF_DEBUG)
endif()

if (${FGE_ENABLE_VALIDATION_LAYERS})
    target_compile_definitions(${FGE_LIB_NAME} PRIVATE FGE_ENABLE_VALIDATION_LAYERS)
    target_compile_definitions(${FGE_SERVER_LIB_NAME} PRIVATE FGE_ENABLE_VALIDATION_LAYERS)
endif()

if (${FGE_ENABLE_SERVER_NETWORK_RANDOM_LOST})
    target_compile_definitions(${FGE_SERVER_LIB_NAME} PRIVATE FGE_ENABLE_SERVER_NETWORK_RANDOM_LOST)
    if (NOT ${FGE_DEBUG})
        message(WARNING "FGE_ENABLE_SERVER_NETWORK_RANDOM_LOST is enabled, but FGE_DEBUG is disabled")
    endif()
endif()

if (${FGE_ENABLE_CLIENT_NETWORK_RANDOM_LOST})
    target_compile_definitions(${FGE_LIB_NAME} PRIVATE FGE_ENABLE_CLIENT_NETWORK_RANDOM_LOST)
    if (NOT ${FGE_DEBUG})
        message(WARNING "FGE_ENABLE_CLIENT_NETWORK_RANDOM_LOST is enabled, but FGE_DEBUG is disabled")
    endif()
endif()

target_compile_definitions(${FGE_SERVER_LIB_NAME} PRIVATE FGE_DEF_SERVER)

#Includes path
target_include_directories(${FGE_LIB_NAME} PUBLIC "includes")
target_include_directories(${FGE_LIB_NAME} PUBLIC "${PROJECT_BINARY_DIR}")

target_include_directories(${FGE_SERVER_LIB_NAME} PUBLIC "includes")
target_include_directories(${FGE_SERVER_LIB_NAME} PUBLIC "${PROJECT_BINARY_DIR}")

#Source files
target_sources(${FGE_SERVER_LIB_NAME} PRIVATE ${FGE_INCLUDES})

target_sources(${FGE_SERVER_LIB_NAME} PRIVATE
        sources/vulkan/vma.cpp
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
        sources/vulkan/C_garbageCollector.cpp
        sources/vulkan/C_context.cpp
        sources/vulkan/C_graphicPipeline.cpp
        sources/vulkan/C_viewport.cpp)

target_sources(${FGE_SERVER_LIB_NAME} PRIVATE
        sources/graphic/C_surface.cpp
        sources/graphic/C_view.cpp
        sources/graphic/C_transformable.cpp
        sources/graphic/C_transform.cpp
        sources/graphic/C_renderWindow.cpp
        sources/graphic/C_renderTexture.cpp
        sources/graphic/C_renderTarget.cpp
        sources/graphic/shaderResources.cpp
        sources/graphic/C_ftFont.cpp)

target_sources(${FGE_SERVER_LIB_NAME} PRIVATE "sources/C_animation.cpp")
target_sources(${FGE_SERVER_LIB_NAME} PRIVATE "sources/C_commandHandler.cpp")
target_sources(${FGE_SERVER_LIB_NAME} PRIVATE "sources/C_event.cpp")
target_sources(${FGE_SERVER_LIB_NAME} PRIVATE "sources/C_eventList.cpp")
target_sources(${FGE_SERVER_LIB_NAME} PRIVATE "sources/C_font.cpp")
target_sources(${FGE_SERVER_LIB_NAME} PRIVATE "sources/C_property.cpp")
target_sources(${FGE_SERVER_LIB_NAME} PRIVATE "sources/C_concavePolygon.cpp")

target_sources(${FGE_SERVER_LIB_NAME} PRIVATE "sources/object/C_objAnim.cpp")
target_sources(${FGE_SERVER_LIB_NAME} PRIVATE "sources/object/C_objButton.cpp")
target_sources(${FGE_SERVER_LIB_NAME} PRIVATE "sources/object/C_object.cpp")
target_sources(${FGE_SERVER_LIB_NAME} PRIVATE "sources/object/C_objectAnchor.cpp")
target_sources(${FGE_SERVER_LIB_NAME} PRIVATE "sources/object/C_objLight.cpp")
target_sources(${FGE_SERVER_LIB_NAME} PRIVATE "sources/object/C_objRenderMap.cpp")
target_sources(${FGE_SERVER_LIB_NAME} PRIVATE "sources/object/C_objSelectBox.cpp")
target_sources(${FGE_SERVER_LIB_NAME} PRIVATE "sources/object/C_objSprite.cpp")
target_sources(${FGE_SERVER_LIB_NAME} PRIVATE "sources/object/C_objSpriteBatches.cpp")
target_sources(${FGE_SERVER_LIB_NAME} PRIVATE "sources/object/C_objSpriteCluster.cpp")
target_sources(${FGE_SERVER_LIB_NAME} PRIVATE "sources/object/C_objSwitch.cpp")
target_sources(${FGE_SERVER_LIB_NAME} PRIVATE "sources/object/C_objText.cpp")
target_sources(${FGE_SERVER_LIB_NAME} PRIVATE "sources/object/C_objTextinputbox.cpp")
target_sources(${FGE_SERVER_LIB_NAME} PRIVATE "sources/object/C_objTilemap.cpp")
target_sources(${FGE_SERVER_LIB_NAME} PRIVATE "sources/object/C_objWindow.cpp")
target_sources(${FGE_SERVER_LIB_NAME} PRIVATE "sources/object/C_objTextList.cpp")
target_sources(${FGE_SERVER_LIB_NAME} PRIVATE "sources/object/C_objSlider.cpp")
target_sources(${FGE_SERVER_LIB_NAME} PRIVATE "sources/object/C_objShape.cpp")
target_sources(${FGE_SERVER_LIB_NAME} PRIVATE "sources/object/C_objRectangleShape.cpp")
target_sources(${FGE_SERVER_LIB_NAME} PRIVATE "sources/object/C_objCircleShape.cpp")
target_sources(${FGE_SERVER_LIB_NAME} PRIVATE "sources/object/C_objLineShape.cpp")

target_sources(${FGE_SERVER_LIB_NAME} PRIVATE "sources/C_guiElement.cpp")

target_sources(${FGE_SERVER_LIB_NAME} PRIVATE "sources/C_random.cpp")

target_sources(${FGE_SERVER_LIB_NAME} PRIVATE "sources/network/C_client.cpp")
target_sources(${FGE_SERVER_LIB_NAME} PRIVATE "sources/network/C_clientList.cpp")
target_sources(${FGE_SERVER_LIB_NAME} PRIVATE "sources/network/C_ipAddress.cpp")
target_sources(${FGE_SERVER_LIB_NAME} PRIVATE "sources/network/C_networkType.cpp")
target_sources(${FGE_SERVER_LIB_NAME} PRIVATE "sources/network/C_packet.cpp")
target_sources(${FGE_SERVER_LIB_NAME} PRIVATE "sources/network/C_packetBZ2.cpp")
target_sources(${FGE_SERVER_LIB_NAME} PRIVATE "sources/network/C_packetLZ4.cpp")
target_sources(${FGE_SERVER_LIB_NAME} PRIVATE "sources/network/C_server.cpp")
target_sources(${FGE_SERVER_LIB_NAME} PRIVATE "sources/network/C_socket.cpp")
target_sources(${FGE_SERVER_LIB_NAME} PRIVATE "sources/C_task.cpp")

target_sources(${FGE_SERVER_LIB_NAME} PRIVATE "sources/object/C_childObjectsAccessor.cpp")
target_sources(${FGE_SERVER_LIB_NAME} PRIVATE "sources/C_scene.cpp")
target_sources(${FGE_SERVER_LIB_NAME} PRIVATE "sources/network/C_sceneUpdateCache.cpp")
target_sources(${FGE_SERVER_LIB_NAME} PRIVATE "sources/C_subscription.cpp")
target_sources(${FGE_SERVER_LIB_NAME} PRIVATE "sources/C_tagList.cpp")
target_sources(${FGE_SERVER_LIB_NAME} PRIVATE "sources/C_texture.cpp")
target_sources(${FGE_SERVER_LIB_NAME} PRIVATE "sources/C_tileset.cpp")
target_sources(${FGE_SERVER_LIB_NAME} PRIVATE "sources/C_tilelayer.cpp")
target_sources(${FGE_SERVER_LIB_NAME} PRIVATE "sources/C_timer.cpp")

target_sources(${FGE_SERVER_LIB_NAME} PRIVATE "sources/extra/extra_function.cpp")
target_sources(${FGE_SERVER_LIB_NAME} PRIVATE "sources/extra/extra_string.cpp")
target_sources(${FGE_SERVER_LIB_NAME} PRIVATE "sources/extra/extra_pathFinding.cpp")

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
target_sources(${FGE_SERVER_LIB_NAME} PRIVATE "sources/manager/shader_manager.cpp")
target_sources(${FGE_SERVER_LIB_NAME} PRIVATE "sources/manager/timer_manager.cpp")
target_sources(${FGE_SERVER_LIB_NAME} PRIVATE "sources/manager/task_manager.cpp")

#######################

target_sources(${FGE_LIB_NAME} PRIVATE ${FGE_INCLUDES})

target_sources(${FGE_LIB_NAME} PRIVATE
        sources/vulkan/vma.cpp
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
        sources/vulkan/C_garbageCollector.cpp
        sources/vulkan/C_context.cpp
        sources/vulkan/C_graphicPipeline.cpp
        sources/vulkan/C_viewport.cpp)

target_sources(${FGE_LIB_NAME} PRIVATE
        sources/graphic/C_surface.cpp
        sources/graphic/C_view.cpp
        sources/graphic/C_transformable.cpp
        sources/graphic/C_transform.cpp
        sources/graphic/C_renderWindow.cpp
        sources/graphic/C_renderTexture.cpp
        sources/graphic/C_renderTarget.cpp
        sources/graphic/shaderResources.cpp
        sources/graphic/C_ftFont.cpp)

target_sources(${FGE_LIB_NAME} PRIVATE "sources/C_animation.cpp")
target_sources(${FGE_LIB_NAME} PRIVATE "sources/C_commandHandler.cpp")
target_sources(${FGE_LIB_NAME} PRIVATE "sources/C_event.cpp")
target_sources(${FGE_LIB_NAME} PRIVATE "sources/C_eventList.cpp")
target_sources(${FGE_LIB_NAME} PRIVATE "sources/C_font.cpp")
target_sources(${FGE_LIB_NAME} PRIVATE "sources/C_property.cpp")
target_sources(${FGE_LIB_NAME} PRIVATE "sources/C_concavePolygon.cpp")

target_sources(${FGE_LIB_NAME} PRIVATE "sources/object/C_objAnim.cpp")
target_sources(${FGE_LIB_NAME} PRIVATE "sources/object/C_objButton.cpp")
target_sources(${FGE_LIB_NAME} PRIVATE "sources/object/C_object.cpp")
target_sources(${FGE_LIB_NAME} PRIVATE "sources/object/C_objectAnchor.cpp")
target_sources(${FGE_LIB_NAME} PRIVATE "sources/object/C_objLight.cpp")
target_sources(${FGE_LIB_NAME} PRIVATE "sources/object/C_objRenderMap.cpp")
target_sources(${FGE_LIB_NAME} PRIVATE "sources/object/C_objSelectBox.cpp")
target_sources(${FGE_LIB_NAME} PRIVATE "sources/object/C_objSprite.cpp")
target_sources(${FGE_LIB_NAME} PRIVATE "sources/object/C_objSpriteBatches.cpp")
target_sources(${FGE_LIB_NAME} PRIVATE "sources/object/C_objSpriteCluster.cpp")
target_sources(${FGE_LIB_NAME} PRIVATE "sources/object/C_objSwitch.cpp")
target_sources(${FGE_LIB_NAME} PRIVATE "sources/object/C_objText.cpp")
target_sources(${FGE_LIB_NAME} PRIVATE "sources/object/C_objTextinputbox.cpp")
target_sources(${FGE_LIB_NAME} PRIVATE "sources/object/C_objTilemap.cpp")
target_sources(${FGE_LIB_NAME} PRIVATE "sources/object/C_objWindow.cpp")
target_sources(${FGE_LIB_NAME} PRIVATE "sources/object/C_objTextList.cpp")
target_sources(${FGE_LIB_NAME} PRIVATE "sources/object/C_objSlider.cpp")
target_sources(${FGE_LIB_NAME} PRIVATE "sources/object/C_objShape.cpp")
target_sources(${FGE_LIB_NAME} PRIVATE "sources/object/C_objRectangleShape.cpp")
target_sources(${FGE_LIB_NAME} PRIVATE "sources/object/C_objCircleShape.cpp")
target_sources(${FGE_LIB_NAME} PRIVATE "sources/object/C_objLineShape.cpp")

target_sources(${FGE_LIB_NAME} PRIVATE "sources/C_guiElement.cpp")

target_sources(${FGE_LIB_NAME} PRIVATE "sources/C_random.cpp")

target_sources(${FGE_LIB_NAME} PRIVATE "sources/network/C_client.cpp")
target_sources(${FGE_LIB_NAME} PRIVATE "sources/network/C_clientList.cpp")
target_sources(${FGE_LIB_NAME} PRIVATE "sources/network/C_ipAddress.cpp")
target_sources(${FGE_LIB_NAME} PRIVATE "sources/network/C_networkType.cpp")
target_sources(${FGE_LIB_NAME} PRIVATE "sources/network/C_packet.cpp")
target_sources(${FGE_LIB_NAME} PRIVATE "sources/network/C_packetBZ2.cpp")
target_sources(${FGE_LIB_NAME} PRIVATE "sources/network/C_packetLZ4.cpp")
target_sources(${FGE_LIB_NAME} PRIVATE "sources/network/C_server.cpp")
target_sources(${FGE_LIB_NAME} PRIVATE "sources/network/C_socket.cpp")
target_sources(${FGE_LIB_NAME} PRIVATE "sources/C_task.cpp")

target_sources(${FGE_LIB_NAME} PRIVATE "sources/object/C_childObjectsAccessor.cpp")
target_sources(${FGE_LIB_NAME} PRIVATE "sources/C_scene.cpp")
target_sources(${FGE_LIB_NAME} PRIVATE "sources/network/C_sceneUpdateCache.cpp")
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

target_sources(${FGE_LIB_NAME} PRIVATE "sources/fge_endian.cpp")

target_sources(${FGE_LIB_NAME} PRIVATE "sources/manager/anim_manager.cpp")
target_sources(${FGE_LIB_NAME} PRIVATE "sources/manager/audio_manager.cpp")
#target_sources(${FGE_LIB_NAME} PRIVATE "sources/manager/crash_manager.cpp") TODO
target_sources(${FGE_LIB_NAME} PRIVATE "sources/manager/font_manager.cpp")
target_sources(${FGE_LIB_NAME} PRIVATE "sources/manager/log_manager.cpp")
target_sources(${FGE_LIB_NAME} PRIVATE "sources/manager/network_manager.cpp")
target_sources(${FGE_LIB_NAME} PRIVATE "sources/manager/path_manager.cpp")
target_sources(${FGE_LIB_NAME} PRIVATE "sources/manager/reg_manager.cpp")
#target_sources(${FGE_LIB_NAME} PRIVATE "sources/manager/screen_manager.cpp") TODO
target_sources(${FGE_LIB_NAME} PRIVATE "sources/manager/texture_manager.cpp")
target_sources(${FGE_LIB_NAME} PRIVATE "sources/manager/shader_manager.cpp")
target_sources(${FGE_LIB_NAME} PRIVATE "sources/manager/timer_manager.cpp")
target_sources(${FGE_LIB_NAME} PRIVATE "sources/manager/task_manager.cpp")

#Linking library
if(WIN32)
    target_link_libraries(${FGE_LIB_NAME} user32 ws2_32 winmm)
    target_link_libraries(${FGE_SERVER_LIB_NAME} user32 ws2_32 winmm)
endif()

target_link_libraries(${FGE_LIB_NAME} glslang::glslang-default-resource-limits glslang::SPIRV glslang::glslang)
target_link_libraries(${FGE_LIB_NAME} SDL2::SDL2 SDL2_image::SDL2_image SDL2_mixer::SDL2_mixer)
target_link_libraries(${FGE_LIB_NAME} BZip2::bz2_static)
target_link_libraries(${FGE_LIB_NAME} LZ4::lz4_static)
target_link_libraries(${FGE_LIB_NAME} fmt::fmt-header-only)
target_link_libraries(${FGE_LIB_NAME} Freetype::Freetype)
target_link_libraries(${FGE_LIB_NAME} re2::re2)
target_link_libraries(${FGE_LIB_NAME} glm::glm-header-only volk::volk_headers GPUOpen::VulkanMemoryAllocator Vulkan::Headers)

target_link_libraries(${FGE_SERVER_LIB_NAME} glslang::glslang-default-resource-limits glslang::SPIRV glslang::glslang)
target_link_libraries(${FGE_SERVER_LIB_NAME} SDL2::SDL2 SDL2_image::SDL2_image)
target_link_libraries(${FGE_SERVER_LIB_NAME} BZip2::bz2_static)
target_link_libraries(${FGE_SERVER_LIB_NAME} LZ4::lz4_static)
target_link_libraries(${FGE_SERVER_LIB_NAME} fmt::fmt-header-only)
target_link_libraries(${FGE_SERVER_LIB_NAME} Freetype::Freetype)
target_link_libraries(${FGE_SERVER_LIB_NAME} re2::re2)
target_link_libraries(${FGE_SERVER_LIB_NAME} glm::glm-header-only volk::volk_headers GPUOpen::VulkanMemoryAllocator Vulkan::Headers)

#Install
include(GNUInstallDirs)

install(TARGETS ${FGE_LIB_NAME})
install(TARGETS ${FGE_SERVER_LIB_NAME})
install(FILES ${FGE_INCLUDES_EXTRA}
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
install(FILES ${FGE_INCLUDES_PROJECT}
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/FastEngine)

#Custom target for executables that copy compiled DLLs and symlink the resources folder
if (MSVC)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/$<CONFIG>)
else()
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
endif()

add_custom_target(FgeClientExeDeps)
add_custom_target(FgeServerExeDeps)
add_dependencies(FgeClientExeDeps ${FGE_LIB_NAME})
add_dependencies(FgeServerExeDeps ${FGE_SERVER_LIB_NAME})

function(CopyTargetDllsToRuntimeDirectory TARGET_NAMES MAIN_TARGET_NAME)
    foreach (TARGET_NAME ${TARGET_NAMES})
        add_custom_command(TARGET ${MAIN_TARGET_NAME} PRE_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                $<TARGET_FILE:${TARGET_NAME}>
                ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
    endforeach()
endfunction()

function(CopyFilesToRuntimeDirectory FILE_PATHS MAIN_TARGET_NAME)
    foreach (FILE_PATH ${FILE_PATHS})
        add_custom_command(TARGET ${MAIN_TARGET_NAME} PRE_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                ${FILE_PATH}
                ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
    endforeach()
endfunction()

if (WIN32)
    CopyTargetDllsToRuntimeDirectory("SDL2::SDL2;SDL2_image::SDL2_image;SDL2_mixer::SDL2_mixer" FgeClientExeDeps)
    CopyTargetDllsToRuntimeDirectory("SDL2::SDL2;SDL2_image::SDL2_image" FgeServerExeDeps)

    CopyTargetDllsToRuntimeDirectory("glslang::glslang-default-resource-limits;glslang::SPIRV;glslang::glslang" FgeClientExeDeps)
    CopyTargetDllsToRuntimeDirectory("glslang::glslang-default-resource-limits;glslang::SPIRV;glslang::glslang" FgeServerExeDeps)
elseif(APPLE)
    #CopyFilesToRuntimeDirectory("${GLSLANG_LIB_APPLE_HELL};${GLSLANG_RES_LIB_APPLE_HELL};${GLSLANG_SPIRV_LIB_APPLE_HELL}" FgeClientExeDeps)
    #CopyFilesToRuntimeDirectory("${GLSLANG_LIB_APPLE_HELL};${GLSLANG_RES_LIB_APPLE_HELL};${GLSLANG_SPIRV_LIB_APPLE_HELL}" FgeServerExeDeps)
endif()

add_custom_target(ResourcesSymlink
        COMMAND ${CMAKE_COMMAND} -E create_symlink
        ${CMAKE_CURRENT_SOURCE_DIR}/resources
        ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/resources
        BYPRODUCTS resources/
        VERBATIM)

add_dependencies(FgeClientExeDeps ResourcesSymlink)
add_dependencies(FgeServerExeDeps ResourcesSymlink)

#Set some helper variables for executables
set(FGE_CLIENT_LIBS ${FGE_SDL_MAIN} SDL2::SDL2 SDL2_image::SDL2_image SDL2_mixer::SDL2_mixer ${FGE_LIB_NAME})
set(FGE_SERVER_LIBS SDL2::SDL2 SDL2_image::SDL2_image ${FGE_SERVER_LIB_NAME})

#Tests
if (FGE_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

function(setMSVCDefaultWorkingDir TARGET)
    set_target_properties(
        ${TARGET}
        PROPERTIES
        VS_DEBUGGER_WORKING_DIRECTORY
        "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")
endfunction()

#Examples
if (FGE_BUILD_EXAMPLES)
    add_subdirectory(examples/tileMapAndPathFinding_001)
    add_subdirectory(examples/lightAndObstacle_002)
    add_subdirectory(examples/guiWindow_003)
    add_subdirectory(examples/clientServerLifeSimulator_004)
    add_subdirectory(examples/deepText_005)
    add_subdirectory(examples/multipleSprites_006)
endif()
