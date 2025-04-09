# SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: GPL-3.0-or-later

cmake_minimum_required(VERSION 3.10)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")

project(dock-searchitem)

FILE(GLOB_RECURSE SRCS
    "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.json"
)

find_package(Qt6 REQUIRED COMPONENTS LinguistTools)
find_package(DDEShell REQUIRED)

add_library(dock-searchitem SHARED ${SRCS})

target_link_libraries(dock-searchitem PRIVATE
    Dde::Shell
)

ds_install_package(PACKAGE org.deepin.ds.dock.searchitem TARGET dock-searchitem)
ds_handle_package_translation(PACKAGE org.deepin.ds.dock.searchitem)

install(FILES "package/icons/search.dci" DESTINATION share/dde-dock/icons/dcc-setting RENAME dcc-search.dci)
