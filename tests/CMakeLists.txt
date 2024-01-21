include(FetchContent)

function(fge_add_test TARGET SOURCES DEPENDS)
    add_executable(${TARGET} ${SOURCES})
    target_link_libraries(${TARGET} PRIVATE ${DEPENDS} fgeDoctestMain)
    add_test(NAME ${TARGET} COMMAND ${TARGET} WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})

    add_dependencies(${TARGET} FgeClientExeDeps)
endfunction()

set(DOCTEST_NO_INSTALL ON)
FetchContent_Declare(doctest
    GIT_REPOSITORY "https://github.com/doctest/doctest.git"
    GIT_TAG v2.4.11
)
FetchContent_MakeAvailable(doctest)

add_library(fgeDoctestMain STATIC doctestMain.cpp)
target_link_libraries(fgeDoctestMain PUBLIC doctest::doctest)

set(TESTS_DEPENDENCIES
    SDL2::SDL2
    SDL2_image::SDL2_image
    SDL2_mixer::SDL2_mixer
    ${FGE_LIB_NAME}
)

fge_add_test(fgeMatrixTests test_fge_matrix.cpp "${TESTS_DEPENDENCIES}")
fge_add_test(fgeExtraStringTests test_fge_extra_string.cpp "${TESTS_DEPENDENCIES}")
fge_add_test(fgeCallbackTests test_fge_callback.cpp "${TESTS_DEPENDENCIES}")
