# 定义需要的cmake版本
cmake_minimum_required(VERSION 3.10)

# 设置工程名字
project(dde-grand-search)

# Install settings
if (CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(CMAKE_INSTALL_PREFIX /usr)
endif ()

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DQT_DEBUG")

configure_file(${CMAKE_SOURCE_DIR}/src/environments.h.in ${CMAKE_SOURCE_DIR}/src/environments.h @ONLY)

include_directories(${CMAKE_SOURCE_DIR}/src)

# 平台库目录
set(LIB_PLATFORM_DIR "/usr/lib/${CMAKE_LIBRARY_ARCHITECTURE}")

#全局文件
FILE(GLOB GLOBAL_SRC "./src/global/*.h" "./src/global/*.cpp")

add_subdirectory(src/grand-search)
add_subdirectory(src/grand-search-daemon)


