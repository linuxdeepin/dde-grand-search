project(image-preview)

# 定义插件名称
set(LIB_NAME image-preview-plugin)

set(SRCS
    imagepreview_global.h
    imagepreviewinterface.h
    imagepreviewinterface.cpp
    imagepreviewplugin.h
    imagepreviewplugin.cpp
    imageview.h
    imageview.cpp
)

add_library(${LIB_NAME} SHARED ${SRCS} ${QRCS})

target_link_libraries(${LIB_NAME} PRIVATE
    ${Qt_LIBS}
    ${DTK_LIBS}
    ${DtkGUI_LIBRARIES}
)

# bin
install(TARGETS ${LIB_NAME} DESTINATION ${PLUGIN_PREVIEW_DIR})

# conf
install(FILES image-preview-plugin.conf DESTINATION ${PLUGIN_PREVIEW_DIR})

