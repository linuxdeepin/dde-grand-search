# SPDX-FileCopyrightText: 2026 testerxydw
#
# SPDX-License-Identifier: GPL-3.0-or-later

# 定义需要的cmake版本
cmake_minimum_required(VERSION 3.10)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Wall -Wl,--as-need -fPIE")

# 定义可执行程序名称
set(BIN_NAME test-convert-search-plugin)

# 依赖包
find_package(${QT_NS} COMPONENTS
    Core
    Gui
    DBus
    Network
REQUIRED)

set(PLUGIN_SRC_PATH "${PROJECT_SOURCE_PATH}/convert-search-plugin")
include_directories(${GTEST_INCLUDE_DIR} ${GTEST_INCLUDE_DIRS})
include_directories(${PLUGIN_SRC_PATH} ${PLUGIN_SRC_PATH}/providers)

# 被测源码：插件实现全量纳入以统计覆盖率；main.cpp 自带入口，排除避免与测试入口冲突
FILE(GLOB_RECURSE PRO_SRC "${PLUGIN_SRC_PATH}/*.h" "${PLUGIN_SRC_PATH}/*.cpp")
list(REMOVE_ITEM PRO_SRC "${PLUGIN_SRC_PATH}/main.cpp")

# 单元测试文件
FILE(GLOB_RECURSE UT_SRC "./*/*.cpp")

set(SRCS
    main.cpp
    ${UT_SRC}
    ${PRO_SRC}
)

add_executable(${BIN_NAME} ${SRCS})

target_link_libraries(${BIN_NAME} PRIVATE
    ${QT_NS}::Core
    ${QT_NS}::Gui
    ${QT_NS}::DBus
    ${QT_NS}::Network
    ${GTEST_LIBRARIES}
    ${QT_NS}::Test
    -lpthread
)
