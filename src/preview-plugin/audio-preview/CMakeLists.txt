project(audio-preview)

# 定义插件名称
set(LIB_NAME audio-preview-plugin)
find_package(PkgConfig REQUIRED)
pkg_check_modules(TAGLIB REQUIRED taglib)
pkg_check_modules(AVFORMAT REQUIRED libavformat)
pkg_check_modules(ICU REQUIRED icu-i18n)

set(SRCS
    audiopreview_global.h
    audiopreviewinterface.h
    audiopreviewinterface.cpp
    audiopreviewplugin.h
    audiopreviewplugin.cpp
    audiofileinfo.h
    audiofileinfo.cpp
    audioview.h
    audioview.cpp
)

add_library(${LIB_NAME} SHARED ${SRCS})

target_include_directories(${LIB_NAME} PUBLIC
    ${TAGLIB_INCLUDE_DIRS}
    ${AVFORMAT_INCLUDE_DIRS}
)

set(Qt_CURRENT_LIBS ${Qt_LIBS})
if (QT_VERSION_MAJOR EQUAL 6)
    find_package(${QT_NS} REQUIRED COMPONENTS Core5Compat)
    set(Qt_CURRENT_LIBS ${Qt_LIBS} Qt6::Core5Compat)
endif()

target_link_libraries(${LIB_NAME} PRIVATE
    ${Qt_CURRENT_LIBS}
    ${DTK_LIBS}
    ${TAGLIB_LIBRARIES}
    ${AVFORMAT_LIBRARIES}
    ${ICU_LIBRARIES}
)

# bin
install(TARGETS ${LIB_NAME} DESTINATION ${PLUGIN_PREVIEW_DIR})
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/audio-preview-plugin.conf DESTINATION ${PLUGIN_PREVIEW_DIR})
