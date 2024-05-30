cmake_minimum_required(VERSION 3.23)

project(DependenciesHandler)

include(ExternalProject)
include(FindGit)

find_package(Git REQUIRED)

set(DEPS_GLOBAL_CMAKE_FILE "${CMAKE_CURRENT_BINARY_DIR}/libs/Dependencies.cmake")
set(CMAKE_INSTALL_PREFIX "${CMAKE_CURRENT_BINARY_DIR}/install/" CACHE PATH "" FORCE)

file(WRITE ${DEPS_GLOBAL_CMAKE_FILE} "")

function(AddCmakeDependency NAME URL TAG SHARED ARGS SOURCE_SUBDIR)
    separate_arguments(ARGS UNIX_COMMAND "${ARGS}")

    ExternalProject_Add(${NAME}
            GIT_REPOSITORY ${URL}
            GIT_TAG ${TAG}
            CMAKE_ARGS -DCMAKE_BUILD_TYPE=Release -DBUILD_SHARED_LIBS=${SHARED} ${ARGS}
            CMAKE_ARGS -DCMAKE_INSTALL_PREFIX:PATH=${CMAKE_CURRENT_BINARY_DIR}/libs/${NAME}
            PREFIX project/${NAME}
            TEST_COMMAND ""
            SOURCE_SUBDIR ${SOURCE_SUBDIR}
            EXCLUDE_FROM_ALL)

    ExternalProject_Get_Property(${NAME} INSTALL_DIR)
    message("Install dir for ${NAME} : ${INSTALL_DIR}")
    
    file(APPEND ${DEPS_GLOBAL_CMAKE_FILE} "find_package(${NAME} REQUIRED NO_DEFAULT_PATH PATHS \"\$\{CMAKE_CURRENT_LIST_DIR\}/${NAME}\")\n")
endfunction()

function(AddPatchedDependency NAME URL TAG SHARED ARGS SOURCE_SUBDIR PATCH)
    separate_arguments(ARGS UNIX_COMMAND "${ARGS}")

    ExternalProject_Add(${NAME}
            GIT_REPOSITORY ${URL}
            GIT_TAG ${TAG}
            CMAKE_ARGS -DCMAKE_BUILD_TYPE=Release -DBUILD_SHARED_LIBS=${SHARED} ${ARGS}
            CMAKE_ARGS -DCMAKE_INSTALL_PREFIX:PATH=${CMAKE_CURRENT_BINARY_DIR}/libs/${NAME}
            PREFIX project/${NAME}
            PATCH_COMMAND ${GIT_EXECUTABLE} clean --force -d -x &&
            PATCH_COMMAND ${GIT_EXECUTABLE} reset --hard &&
            PATCH_COMMAND ${GIT_EXECUTABLE} apply "${PATCH}" --ignore-whitespace
            TEST_COMMAND ""
            SOURCE_SUBDIR ${SOURCE_SUBDIR}
            EXCLUDE_FROM_ALL)

    ExternalProject_Get_Property(${NAME} INSTALL_DIR)
    message("Install dir for ${NAME} : ${INSTALL_DIR}")
    
    file(APPEND ${DEPS_GLOBAL_CMAKE_FILE} "find_package(${NAME} REQUIRED NO_DEFAULT_PATH PATHS \"\$\{CMAKE_CURRENT_LIST_DIR\}/${NAME}/*\")\n")
endfunction()

function(AddSingleHeaderDependency NAME URL TAG HEADER_PATH)
    get_filename_component(HEADER_PATH_NAME ${HEADER_PATH} NAME)

    ExternalProject_Add(${NAME}
            GIT_REPOSITORY ${URL}
            GIT_TAG ${TAG}
            PREFIX project/${NAME}
            BUILD_IN_SOURCE ON
            CONFIGURE_COMMAND ""
            BUILD_COMMAND ""
            INSTALL_COMMAND ${CMAKE_COMMAND} -E copy_if_different "${HEADER_PATH}" "${CMAKE_CURRENT_BINARY_DIR}/libs/${NAME}/include/${HEADER_PATH_NAME}"
            TEST_COMMAND ""
            EXCLUDE_FROM_ALL)

    ExternalProject_Get_Property(${NAME} INSTALL_DIR)
    message("Install dir for ${NAME} : ${INSTALL_DIR}")
    
    file(APPEND ${DEPS_GLOBAL_CMAKE_FILE} "set(DEPS_${NAME}_HEADER_FILE \"\$\{CMAKE_CURRENT_LIST_DIR\}/${NAME}/include/${HEADER_PATH_NAME}\")\n")
endfunction()

AddPatchedDependency(bzip2
        https://gitlab.com/bzip2/bzip2.git
        66c46b8c9436613fd81bc5d03f63a61933a4dcc3
        OFF
        "-DENABLE_LIB_ONLY=ON -DENABLE_APP=OFF -DENABLE_STATIC_LIB=ON"
        ""
        "${CMAKE_CURRENT_SOURCE_DIR}/patches/0001-Bzip2-cmake-package.patch")

AddCmakeDependency(fmt
        https://github.com/fmtlib/fmt.git
        10.2.1
        OFF
        "-DFMT_TEST=OFF"
        "")

AddCmakeDependency(lz4
        https://github.com/lz4/lz4.git
        v1.9.4
        OFF
        "-DLZ4_BUILD_CLI=OFF -DLZ4_BUILD_LEGACY_LZ4C=OFF"
        "build/cmake")

AddCmakeDependency(re2
        https://github.com/google/re2.git
        2023-03-01
        OFF
        "-DRE2_BUILD_TESTING=OFF -DBUILD_TESTING=OFF -DCMAKE_POSITION_INDEPENDENT_CODE=ON"
        "")

AddCmakeDependency(freetype
        https://gitlab.freedesktop.org/freetype/freetype.git
        VER-2-13-2
        OFF
        "-DFT_DISABLE_HARFBUZZ=ON -DFT_DISABLE_ZLIB=ON -DFT_DISABLE_PNG=ON -DFT_DISABLE_BROTLI=ON -DFT_DISABLE_BZIP2=ON -DCMAKE_POSITION_INDEPENDENT_CODE=ON"
        "")

AddCmakeDependency(SDL2
        https://github.com/libsdl-org/SDL.git
        release-2.30.3
        ON
        "-DSDL_TEST=OFF -DSDL_STATIC_PIC=ON"
        "")

AddCmakeDependency(SDL2_image
        https://github.com/libsdl-org/SDL_image.git
        release-2.8.2
        ON
        "-DSDL2IMAGE_SAMPLES=OFF -DSDL2IMAGE_DEPS_SHARED=OFF -DSDL2IMAGE_VENDORED=ON -DCMAKE_PREFIX_PATH=${CMAKE_CURRENT_BINARY_DIR}/libs/SDL2"
        "")
add_dependencies(SDL2_image SDL2)

AddCmakeDependency(SDL2_mixer
        https://github.com/libsdl-org/SDL_mixer.git
        release-2.8.0
        ON
        "-DSDL2MIXER_VENDORED=ON -DCMAKE_PREFIX_PATH=${CMAKE_CURRENT_BINARY_DIR}/libs/SDL2"
        "")
add_dependencies(SDL2_mixer SDL2)

AddCmakeDependency(glm
        https://github.com/g-truc/glm.git
        45008b225e28eb700fa0f7d3ff69b7c1db94fadf
        OFF
        "-DGLM_BUILD_LIBRARY=OFF -DGLM_BUILD_TESTS=OFF"
        "")

AddCmakeDependency(VulkanHeaders
        https://github.com/KhronosGroup/Vulkan-Headers.git
        vulkan-sdk-1.3.268.0
        OFF
        "-DBUILD_TESTS=OFF"
        "")

AddCmakeDependency(volk
        https://github.com/zeux/volk.git
        443b10b6d4cd4c889565e8478b3e512ee02ea898
        OFF
        "-DVOLK_INSTALL=ON -DVOLK_HEADERS_ONLY=ON -DVOLK_PULL_IN_VULKAN=ON -DVULKAN_HEADERS_INSTALL_DIR=${CMAKE_CURRENT_BINARY_DIR}/libs/VulkanHeaders"
        "")
add_dependencies(volk VulkanHeaders)

AddCmakeDependency(VulkanMemoryAllocator
        https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator.git
        eaf8fc27eeadf6f21b11183651b829e897f01957
        OFF
        "-DVMA_BUILD_DOCUMENTATION=OFF -DVMA_BUILD_SAMPLES=OFF"
        "")

AddCmakeDependency(glslang
        https://github.com/KhronosGroup/glslang.git
        14.2.0
        OFF
        "-DBUILD_TESTING=OFF -DENABLE_OPT=OFF -DENABLE_GLSLANG_BINARIES=OFF"
        "")
