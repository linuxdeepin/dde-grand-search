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

# 国际化
if (NOT (${CMAKE_BUILD_TYPE} MATCHES "Debug"))
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Ofast")

    # generate qm
    execute_process(COMMAND bash "translate_generation.sh"
                    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
endif ()

# 定义前端可执行程序名称
set(SEARCH_BIN_NAME dde-grand-search)

# 定义前端应用库目录
set(SEARCH_LIB_BASE_DIR "${LIB_PLATFORM_DIR}/${SEARCH_BIN_NAME}")

# 定义前端插件目录
set(SEARCH_PLUGIN_BASE_DIR "${SEARCH_LIB_BASE_DIR}/plugins")

# 定义前端预览插件目录
set(PLUGIN_PREVIEW_DIR "${SEARCH_PLUGIN_BASE_DIR}/preview")
add_definitions(-DPLUGIN_PREVIEW_DIR="${PLUGIN_PREVIEW_DIR}")

# 定义后端可执行程序名称
set(DAEMON_BIN_NAME dde-grand-search-daemon)

# 定义后端应用库目录
set(DAEMON_LIB_BASE_DIR "${LIB_PLATFORM_DIR}/${DAEMON_BIN_NAME}")

# 定义后端插件目录
set(DAEMON_PLUGIN_DIR "${DAEMON_LIB_BASE_DIR}/plugins")

# 定义后端扩展搜索插件目录
set(PLUGIN_SEARCHER_DIR "${DAEMON_PLUGIN_DIR}/searcher")
add_definitions(-DPLUGIN_SEARCHER_DIR="${PLUGIN_SEARCHER_DIR}")

add_subdirectory(src/grand-search)
add_subdirectory(src/grand-search-daemon)
add_subdirectory(src/grand-search-dock-plugin)
add_subdirectory(src/preview-plugin)
