# 定义需要的cmake版本
cmake_minimum_required(VERSION 3.10)

# 设置工程名字
project(test-dde-grand-search)

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DQT_DEBUG")

set(PROJECT_SOURCE_PATH "${CMAKE_SOURCE_DIR}/../src")
set(PROJECT_3RDPARTY_PATH "${CMAKE_SOURCE_DIR}/../3rdparty")
set(TESTS_3RDPARTY_PATH "${CMAKE_SOURCE_DIR}/3rdparty")

# 内存泄漏
if (NOT ("${CMAKE_SYSTEM_PROCESSOR}" MATCHES "mips"))
    ADD_DEFINITIONS(-DENABLE_SANITIZER)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=undefined,address,leak -fno-omit-frame-pointer")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fsanitize=undefined,address,leak -fno-omit-frame-pointer")
endif()

# 用于测试覆盖率的编译条件
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-inline -fno-access-control -O0 -fprofile-arcs -ftest-coverage -lgcov")

# Qt 版本自动探测（仿照根 CMakeLists.txt）
if (NOT DEFINED QT_VERSION_MAJOR)
    find_package(Qt6 QUIET COMPONENTS Core)
    if (Qt6_FOUND)
        set(QT_VERSION_MAJOR 6)
        message("   >>> Auto detected Qt version: 6")
    else()
        find_package(Qt5 QUIET COMPONENTS Core)
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

if (QT_VERSION_MAJOR EQUAL 6)
    set(DTK_VERSION_MAJOR 6)
else()
    set(DTK_VERSION_MAJOR "")
endif()
message("   >>> Found Qt version: ${QT_VERSION_MAJOR}")
message("   >>> Build with DTK: ${DTK_VERSION_MAJOR}")

set(QT_NS Qt${QT_VERSION_MAJOR})

# DTK 包名 / 目标名变量（供子目录 CMakeLists.txt 使用）
if (QT_VERSION_MAJOR EQUAL 6)
    set(DTK_WIDGET_PKG Dtk6Widget)
    set(DTK_GUI_PKG    Dtk6Gui)
    set(DTK_CORE_PKG   Dtk6Core)
    set(DTK_CMAKE_PKG  Dtk6CMake)
else()
    set(DTK_WIDGET_PKG DtkWidget)
    set(DTK_GUI_PKG    DtkGui)
    set(DTK_CORE_PKG   DtkCore)
    set(DTK_CMAKE_PKG  DtkCMake)
endif()

# UT依赖
find_package(GTest REQUIRED)
find_package(${QT_NS} COMPONENTS Test REQUIRED)

# Qt6 需要将 Qt6 cmake 目录加入 CMAKE_MODULE_PATH（用于 FindWrapOpenGL 等）
if (QT_VERSION_MAJOR EQUAL 6 AND DEFINED Qt6_DIR)
    list(APPEND CMAKE_MODULE_PATH "${Qt6_DIR}")
    # Qt6Gui 依赖 WrapOpenGL → 需要 OpenGL
    find_package(OpenGL QUIET)
    # Qt6 下 QTextCodec 移至 Core5Compat 模块
    find_package(${QT_NS} COMPONENTS Core5Compat QUIET)
endif()

# 定义 APP_VERSION（根 CMakeLists.txt 通过 add_compile_definitions 定义，测试需手动设置）
if (NOT DEFINED APP_VERSION)
    set(APP_VERSION "0.0.1")
endif()
add_definitions(-DAPP_VERSION="${APP_VERSION}")

# 全局文件
FILE(GLOB_RECURSE GLOBAL_SRC "${PROJECT_SOURCE_PATH}/global/*.h" "${PROJECT_SOURCE_PATH}/global/*.cpp")

# 打桩工具
FILE(GLOB CPP_STUB_SRC "${TESTS_3RDPARTY_PATH}/cpp-stub/*.h"
    "${TESTS_3RDPARTY_PATH}/cpp-stub/*.hpp"
    "${TESTS_3RDPARTY_PATH}/stub-ext/*.h"
    "${TESTS_3RDPARTY_PATH}/stub-ext/*.cpp")

include_directories("${TESTS_3RDPARTY_PATH}/cpp-stub" "${TESTS_3RDPARTY_PATH}/stub-ext")
include_directories(${PROJECT_SOURCE_PATH})
include_directories("${CMAKE_SOURCE_DIR}/../3rdparty")

add_subdirectory(grand-search)
# FIXME Qt6: libgrand-search-daemon 源码目录结构变更、缺少头文件、
# stub-ext 对 noexcept 函数指针不兼容，暂时跳过此模块
# add_subdirectory(libgrand-search-daemon)
add_subdirectory(preview-plugin)
add_subdirectory(grand-search-dock-plugin)

