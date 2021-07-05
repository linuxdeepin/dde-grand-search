# 定义需要的cmake版本
cmake_minimum_required(VERSION 3.10)

# 设置工程名字
project(dde-grand-search)

# Install settings
if (CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(CMAKE_INSTALL_PREFIX /usr)
endif ()

configure_file(${CMAKE_SOURCE_DIR}/src/environments.h.in ${CMAKE_SOURCE_DIR}/src/environments.h @ONLY)

add_subdirectory(src/grand-search)
add_subdirectory(src/grand-search-daemon)


