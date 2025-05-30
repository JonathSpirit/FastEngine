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

function(AddPatchedDependency NAME URL TAG SHARED ARGS SOURCE_SUBDIR PATCH APPEND_ASTERISK)
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
    
    if (APPEND_ASTERISK)
        file(APPEND ${DEPS_GLOBAL_CMAKE_FILE} "find_package(${NAME} REQUIRED NO_DEFAULT_PATH PATHS \"\$\{CMAKE_CURRENT_LIST_DIR\}/${NAME}/*\")\n")
    else()
        file(APPEND ${DEPS_GLOBAL_CMAKE_FILE} "find_package(${NAME} REQUIRED NO_DEFAULT_PATH PATHS \"\$\{CMAKE_CURRENT_LIST_DIR\}/${NAME}\")\n")
    endif()
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
        "${CMAKE_CURRENT_SOURCE_DIR}/patches/0001-Bzip2-cmake-package.patch"
        ON)

AddCmakeDependency(fmt
        https://github.com/fmtlib/fmt.git
        11.2.0
        OFF
        "-DFMT_TEST=OFF"
        "")

AddCmakeDependency(lz4
        https://github.com/lz4/lz4.git
        v1.10.0
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
        VER-2-13-3
        OFF
        "-DFT_DISABLE_HARFBUZZ=ON -DFT_DISABLE_ZLIB=ON -DFT_DISABLE_PNG=ON -DFT_DISABLE_BROTLI=ON -DFT_DISABLE_BZIP2=ON -DCMAKE_POSITION_INDEPENDENT_CODE=ON"
        "")

AddCmakeDependency(SDL2
        https://github.com/libsdl-org/SDL.git
        release-2.32.6
        ON
        "-DSDL_TEST=OFF -DSDL_STATIC_PIC=ON"
        "")

AddCmakeDependency(SDL2_image
        https://github.com/libsdl-org/SDL_image.git
        release-2.8.8
        ON
        "-DCMAKE_POLICY_VERSION_MINIMUM=3.5 -DSDL2IMAGE_AVIF=OFF -DDAV1D_ASM=OFF -DSDL2IMAGE_SAMPLES=OFF -DSDL2IMAGE_DEPS_SHARED=OFF -DSDL2IMAGE_VENDORED=ON -DCMAKE_PREFIX_PATH=${CMAKE_CURRENT_BINARY_DIR}/libs/SDL2"
        "")
add_dependencies(SDL2_image SDL2)

AddCmakeDependency(SDL2_mixer
        https://github.com/libsdl-org/SDL_mixer.git
        release-2.8.1
        ON
        "-DSDL2MIXER_VENDORED=ON -DCMAKE_PREFIX_PATH=${CMAKE_CURRENT_BINARY_DIR}/libs/SDL2"
        "")
add_dependencies(SDL2_mixer SDL2)

AddCmakeDependency(glm
        https://github.com/g-truc/glm.git
        2d4c4b4dd31fde06cfffad7915c2b3006402322f
        OFF
        "-DGLM_BUILD_LIBRARY=OFF -DGLM_BUILD_TESTS=OFF"
        "")

AddCmakeDependency(VulkanHeaders
        https://github.com/KhronosGroup/Vulkan-Headers.git
        vulkan-sdk-1.3.290.0
        OFF
        "-DBUILD_TESTS=OFF -DVULKAN_HEADERS_ENABLE_MODULE=OFF"
        "")

AddPatchedDependency(volk
        https://github.com/zeux/volk.git
        8a7395421421be5fb79ec84182540bc5934853e6
        OFF
        "-DVOLK_INSTALL=ON -DVOLK_HEADERS_ONLY=ON -DVOLK_PULL_IN_VULKAN=ON -DVULKAN_HEADERS_INSTALL_DIR=${CMAKE_CURRENT_BINARY_DIR}/libs/VulkanHeaders"
        ""
        "${CMAKE_CURRENT_SOURCE_DIR}/patches/0001-volk-avoid_vulkan_headers_paths.patch"
        OFF)
add_dependencies(volk VulkanHeaders)

AddCmakeDependency(VulkanMemoryAllocator
        https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator.git
        871913da6a4b132b567d7b65c509600363c0041e
        OFF
        "-DVMA_BUILD_DOCUMENTATION=OFF -DVMA_BUILD_SAMPLES=OFF"
        "")

AddCmakeDependency(glslang
        https://github.com/KhronosGroup/glslang.git
        15.3.0
        OFF
        "-DBUILD_TESTING=OFF -DENABLE_OPT=OFF -DENABLE_GLSLANG_BINARIES=OFF"
        "")
