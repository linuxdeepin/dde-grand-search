project(text-preview)

# 定义插件名称
set(LIB_NAME text-preview-plugin)

set(SRCS
    textpreview_global.h
    textpreviewinterface.h
    textpreviewinterface.cpp
    textpreviewplugin.h
    textpreviewplugin.cpp
    textview.h
    textview.cpp
)

add_library(${LIB_NAME} SHARED ${SRCS} ${QRCS})

set(Qt_CURRENT_LIBS ${Qt_LIBS})
if (QT_VERSION_MAJOR EQUAL 6)
    find_package(${QT_NS} REQUIRED COMPONENTS Core5Compat)
    set(Qt_CURRENT_LIBS ${Qt_LIBS} Qt6::Core5Compat)
endif()

target_link_libraries(${LIB_NAME} PRIVATE
    ${Qt_CURRENT_LIBS}
    ${DTK_LIBS}
)

# bin
install(TARGETS ${LIB_NAME} DESTINATION ${PLUGIN_PREVIEW_DIR})

# conf
install(FILES text-preview-plugin.conf DESTINATION ${PLUGIN_PREVIEW_DIR})
