add_executable(fgeDeploy deploy/main.cpp)

#fgeDeploy must run on its own, without the compiler runtime DLLs/libraries next to it
if (MINGW)
    #libstdc++, libgcc and winpthread
    target_link_options(fgeDeploy PRIVATE -static)
elseif (MSVC)
    #static CRT (/MT, /MTd)
    set_property(TARGET fgeDeploy PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
elseif (CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang" AND NOT APPLE)
    #a fully static glibc is not recommended, only the C++ runtime is linked statically
    target_link_options(fgeDeploy PRIVATE -static-libstdc++ -static-libgcc)
endif ()

install(TARGETS fgeDeploy RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
