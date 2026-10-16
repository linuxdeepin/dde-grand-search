cmake_minimum_required(VERSION 3.10)
project(convert-search-plugin)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_AUTOMOC ON)

# ── Qt 版本跟随主工程（未定义时自动探测 Qt6/Qt5）─────────────────────
if (NOT DEFINED QT_VERSION_MAJOR)
    find_package(Qt6 QUIET COMPONENTS Core Gui DBus Network)
    if (Qt6_FOUND)
        set(QT_VERSION_MAJOR 6)
    else()
        find_package(Qt5 QUIET COMPONENTS Core Gui DBus Network)
        if (Qt5_FOUND)
            set(QT_VERSION_MAJOR 5)
        else()
            message(FATAL_ERROR "Neither Qt6 nor Qt5 found!")
        endif()
    endif()
endif()

set(QT_NS Qt${QT_VERSION_MAJOR})

find_package(${QT_NS} REQUIRED COMPONENTS Core Gui DBus Network)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

# ── 源文件 ────────────────────────────────────────────────────────────
set(PLUGIN_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/convertsearch.h
    ${CMAKE_CURRENT_SOURCE_DIR}/convertsearch.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/searchpluginadaptor.h
    ${CMAKE_CURRENT_SOURCE_DIR}/searchpluginadaptor.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/resultbuilder.h
    ${CMAKE_CURRENT_SOURCE_DIR}/resultbuilder.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/queryparser.h
    ${CMAKE_CURRENT_SOURCE_DIR}/queryparser.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/i18n.h
    ${CMAKE_CURRENT_SOURCE_DIR}/i18n.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/clipboard.h
    ${CMAKE_CURRENT_SOURCE_DIR}/clipboard.cpp
)

file(GLOB PROVIDER_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/providers/*.h
    ${CMAKE_CURRENT_SOURCE_DIR}/providers/*.cpp
)
list(APPEND PLUGIN_SOURCES ${PROVIDER_SOURCES})

# ── 二进制 ────────────────────────────────────────────────────────────
add_executable(${PROJECT_NAME} ${PLUGIN_SOURCES})

target_link_libraries(${PROJECT_NAME}
    PRIVATE
        ${QT_NS}::Core
        ${QT_NS}::Gui
        ${QT_NS}::DBus
        ${QT_NS}::Network
)

# ── 安装：二进制与 conf 均进 daemon 插件扫描目录 ─────────────────────
include(GNUInstallDirs)

if(NOT DEFINED CONVERT_PLUGIN_DIR)
    set(CONVERT_PLUGIN_DIR
        ${CMAKE_INSTALL_FULL_LIBDIR}/dde-grand-search-daemon/plugins/searcher
    )
endif()

install(TARGETS ${PROJECT_NAME}
    RUNTIME DESTINATION ${CONVERT_PLUGIN_DIR}
)

set(CONVERT_PLUGIN_EXEC "${CONVERT_PLUGIN_DIR}/${PROJECT_NAME}")
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/convert-search.conf.in
    ${CMAKE_CURRENT_BINARY_DIR}/convert-search.conf
    @ONLY
)

install(
    FILES ${CMAKE_CURRENT_BINARY_DIR}/convert-search.conf
    DESTINATION ${CONVERT_PLUGIN_DIR}
)
