cmake_minimum_required(VERSION 3.10)
project(calculator-search-plugin)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_AUTOMOC ON)

# ── Qt version auto-detection (Qt6 first, fallback Qt5) ──────────────
if (NOT DEFINED QT_VERSION_MAJOR)
    find_package(Qt6 QUIET COMPONENTS Core Gui DBus)
    if (Qt6_FOUND)
        set(QT_VERSION_MAJOR 6)
        message("   >>> Auto detected Qt version: 6")
    else()
        find_package(Qt5 QUIET COMPONENTS Core Gui DBus)
        if (Qt5_FOUND)
            set(QT_VERSION_MAJOR 5)
            message("   >>> Auto detected Qt version: 5")
        else()
            message(FATAL_ERROR "Neither Qt6 nor Qt5 found!")
        endif()
    endif()
else()
    message("   >>> Using specified Qt version: ${QT_VERSION_MAJOR}")
endif()

set(QT_NS Qt${QT_VERSION_MAJOR})

find_package(${QT_NS} REQUIRED COMPONENTS Core Gui DBus)

# ── Include paths ────────────────────────────────────────────────────
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/exprtk
)

# ── Sources ──────────────────────────────────────────────────────────
set(PLUGIN_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/calculatorsearch.h
    ${CMAKE_CURRENT_SOURCE_DIR}/calculatorsearch.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/searchpluginadaptor.h
    ${CMAKE_CURRENT_SOURCE_DIR}/searchpluginadaptor.cpp
)

# ── Binary ───────────────────────────────────────────────────────────
add_executable(${PROJECT_NAME} ${PLUGIN_SOURCES})

target_link_libraries(${PROJECT_NAME}
    PRIVATE
        ${QT_NS}::Core
        ${QT_NS}::Gui
        ${QT_NS}::DBus
)

# ── Install ───────────────────────────────────────────────────────────
include(GNUInstallDirs)

# Platform-aware plugin directory
if(NOT DEFINED CALCULATOR_PLUGIN_DIR)
    set(CALCULATOR_PLUGIN_DIR
        ${CMAKE_INSTALL_FULL_LIBDIR}/dde-grand-search-daemon/plugins/searcher
    )
endif()

install(TARGETS ${PROJECT_NAME}
    RUNTIME DESTINATION ${CALCULATOR_PLUGIN_DIR}
)

# Generate plugin configuration file from .in template
set(CALCULATOR_PLUGIN_EXEC "${CALCULATOR_PLUGIN_DIR}/${PROJECT_NAME}")
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/calculator-search.conf.in
    ${CMAKE_CURRENT_BINARY_DIR}/calculator-search.conf
    @ONLY
)

install(
    FILES ${CMAKE_CURRENT_BINARY_DIR}/calculator-search.conf
    DESTINATION ${CALCULATOR_PLUGIN_DIR}
)
