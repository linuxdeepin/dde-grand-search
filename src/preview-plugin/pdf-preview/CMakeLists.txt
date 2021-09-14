project(pdf-preview)

# 定义插件名称
set(LIB_NAME pdf-preview-plugin)

find_package(PkgConfig REQUIRED)
pkg_check_modules(PDF REQUIRED poppler-qt5)

set(SRCS
    pdfpreviewinterface.h
    pdfpreviewinterface.cpp
    pdfpreviewplugin.h
    pdfpreviewplugin.cpp
    pdfview.h
    pdfview.cpp
)

add_library(${LIB_NAME} SHARED ${SRCS})

target_include_directories(${LIB_NAME} PUBLIC
    ${DtkWidget_INCLUDE_DIRS}
    ${DtkGUI_INCLUDE_DIRS}
    ${PDF_INCLUDE_DIRS}
)

target_link_libraries(${LIB_NAME} PRIVATE
    ${Qt_LIBS}
    ${DtkWidget_LIBRARIES}
    ${DtkGUI_LIBRARIES}
    ${PDF_LIBRARIES}
)

# bin
install(TARGETS ${LIB_NAME} DESTINATION ${PLUGIN_PREVIEW_DIR})