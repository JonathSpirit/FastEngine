include(FetchContent)

function(fge_add_test target SOURCES DEPENDS)
    add_executable(${target} ${SOURCES})
    target_link_libraries(${target} PRIVATE ${DEPENDS} fgeDoctestMain)
    add_test(${target} ${target})

    #Copy dlls
    if (WIN32)
        foreach (DEPENDENCY ${DEPENDS})
            add_custom_command(TARGET ${target} PRE_BUILD
                    COMMAND ${CMAKE_COMMAND} -E copy
                    $<TARGET_FILE:${DEPENDENCY}>
                    $<TARGET_FILE_DIR:${target}>)
        endforeach()
    endif()
endfunction()

set(DOCTEST_NO_INSTALL ON)
FetchContent_Declare(doctest
    GIT_REPOSITORY "https://github.com/doctest/doctest.git"
    GIT_TAG v2.4.9
)
FetchContent_MakeAvailable(doctest)

add_library(fgeDoctestMain STATIC doctestMain.cpp)
target_link_libraries(fgeDoctestMain PUBLIC doctest::doctest)

set(TESTS_DEPENDENCIES
    sfml-graphics
    ${FGE_SFML_MAIN}
    sfml-system
    sfml-window
    ${FGE_LIB_NAME}
)

set(TESTS_SRC
    test_fge_matrix.cpp
)
fge_add_test(fgeMatrixTests "${TESTS_SRC}" "${TESTS_DEPENDENCIES}")