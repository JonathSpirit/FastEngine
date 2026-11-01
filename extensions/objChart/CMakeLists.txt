#ObjChart extension (header only)
set(FGE_EXT_OBJCHART_LIB_NAME "${PROJECT_NAME}ObjChart")

add_library(${FGE_EXT_OBJCHART_LIB_NAME} INTERFACE)
add_library(${PROJECT_NAME}::ObjChart ALIAS ${FGE_EXT_OBJCHART_LIB_NAME})
set_target_properties(${FGE_EXT_OBJCHART_LIB_NAME} PROPERTIES EXPORT_NAME ObjChart)

target_sources(${FGE_EXT_OBJCHART_LIB_NAME} INTERFACE FILE_SET publicHeaders TYPE HEADERS
        BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/includes
        FILES
        includes/FastEngine/extensions/C_objChart.hpp
        includes/FastEngine/extensions/C_objChart.inl)

target_link_libraries(${FGE_EXT_OBJCHART_LIB_NAME} INTERFACE ${FGE_LIB_NAME})

install(TARGETS ${FGE_EXT_OBJCHART_LIB_NAME} EXPORT ${PROJECT_NAME}Targets
        FILE_SET publicHeaders DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/${PROJECT_NAME})
