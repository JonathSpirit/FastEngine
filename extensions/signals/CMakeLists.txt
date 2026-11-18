set(FGE_EXT_SIGNALS_LIB_NAME "${PROJECT_NAME}Signals")

add_library(${FGE_EXT_SIGNALS_LIB_NAME} INTERFACE)
add_library(${PROJECT_NAME}::Signals ALIAS ${FGE_EXT_SIGNALS_LIB_NAME})
set_target_properties(${FGE_EXT_SIGNALS_LIB_NAME} PROPERTIES EXPORT_NAME Signals)

target_sources(${FGE_EXT_SIGNALS_LIB_NAME} INTERFACE FILE_SET publicHeaders TYPE HEADERS
        BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/includes
        FILES
        includes/FastEngine/extensions/C_signal.hpp
        includes/FastEngine/extensions/C_signal.inl
        includes/FastEngine/extensions/C_signalGenerator.hpp
        includes/FastEngine/extensions/C_signalGenerator.inl)

target_link_libraries(${FGE_EXT_SIGNALS_LIB_NAME} INTERFACE ${FGE_LIB_NAME})

install(TARGETS ${FGE_EXT_SIGNALS_LIB_NAME} EXPORT ${PROJECT_NAME}Targets
        FILE_SET publicHeaders DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/${PROJECT_NAME})
