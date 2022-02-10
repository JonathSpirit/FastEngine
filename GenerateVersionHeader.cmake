if(GIT_EXECUTABLE)
  get_filename_component(SRC_DIR ${SRC} DIRECTORY)
  # Generate a git-describe version string from Git repository tags
  execute_process(
    COMMAND ${GIT_EXECUTABLE} describe --tags --dirty --match "v*"
    WORKING_DIRECTORY ${SRC_DIR}
    OUTPUT_VARIABLE GIT_DESCRIBE_VERSION
    RESULT_VARIABLE GIT_DESCRIBE_ERROR_CODE
    OUTPUT_STRIP_TRAILING_WHITESPACE
    )
  if(NOT GIT_DESCRIBE_ERROR_CODE)
    STRING(REGEX MATCH "v?((([0-9]+)\\.([0-9]+)\\.([0-9]+))-(.+))" REGEX_OUT ${GIT_DESCRIBE_VERSION})

    # message("match 1 : ${CMAKE_MATCH_1}")
    # message("match 2 : ${CMAKE_MATCH_2}")
    # message("match 3 : ${CMAKE_MATCH_3}")
    # message("match 4 : ${CMAKE_MATCH_4}")
    # message("match 5 : ${CMAKE_MATCH_5}")
    # message("match 6 : ${CMAKE_MATCH_6}")

    set(VAR_FGE_VERSION_FULL_WITHTAG_STRING ${CMAKE_MATCH_1})
    set(VAR_FGE_VERSION_FULL_STRING ${CMAKE_MATCH_2})

    set(VAR_FGE_VERSION_MAJOR ${CMAKE_MATCH_3})
    set(VAR_FGE_VERSION_MINOR ${CMAKE_MATCH_4})
    set(VAR_FGE_VERSION_REVISION ${CMAKE_MATCH_5})

    set(VAR_FGE_VERSION_GITTAG ${CMAKE_MATCH_6})
  else()
    set(VAR_FGE_VERSION_FULL_WITHTAG_STRING 0.0.0-unknown)
    set(VAR_FGE_VERSION_FULL_STRING 0.0.0)

    set(VAR_FGE_VERSION_MAJOR 0)
    set(VAR_FGE_VERSION_MINOR 0)
    set(VAR_FGE_VERSION_REVISION 0)

    set(VAR_FGE_VERSION_GITTAG unknown)

    message(WARNING "Failed to determine version from Git tags. Using default version \"${VAR_FGE_VERSION_FULL_WITHTAG_STRING}\".")
  endif()
endif()

configure_file(${SRC} ${DST} @ONLY)