add_executable(fgeDeploy deploy/main.cpp)
install(TARGETS fgeDeploy RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
