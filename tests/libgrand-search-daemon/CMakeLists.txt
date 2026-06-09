# 定义需要的cmake版本
cmake_minimum_required(VERSION 3.10)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Wall -Wl,--as-need -fPIE")

# 应用库目录
set(LIB_BASE_DIR "${LIB_PLATFORM_DIR}/${BIN_NAME}")

# 插件目录
set(PLUGIN_DIR "${LIB_BASE_DIR}/plugins")

# 扩展搜索插件目录
set(PLUGIN_SEARCHER_DIR "${PLUGIN_DIR}/searcher")
add_definitions(-DPLUGIN_SEARCHER_DIR="${PLUGIN_SEARCHER_DIR}")

# 定义可执行程序名称
set(BIN_NAME test-libdde-grand-search-daemon)

# 依赖包
find_package(PkgConfig REQUIRED)
find_package(${DTK_CORE_PKG} REQUIRED)
#find_package(DtkCMake REQUIRED)
find_package(${QT_NS} COMPONENTS
    Core
    Gui
    Concurrent
    DBus
REQUIRED)

set(PROJECT_DAEMONSRC_PATH "${PROJECT_SOURCE_PATH}/libgrand-search-daemon")
set(PROJECT_TOOLS_PATH "${PROJECT_SOURCE_PATH}/tools")
include_directories(${PROJECT_DAEMONSRC_PATH})
include_directories(${PROJECT_TOOLS_PATH})

# ENABLE_FSEARCH / ENABLE_DEEPINANYTHING 不再使用：源码无引用，
# 3rdparty/fsearch 和 anything_interface 已移除，注释掉以避免编译缺失文件
# ADD_DEFINITIONS(-DENABLE_FSEARCH)

# FIXME: AnalyzeServer.xml 已从 3rdparty 移除，注释掉 dbus 接口生成
# if (QT_VERSION_MAJOR EQUAL 6)
#     qt6_add_dbus_interface(IFS_SRC ${PROJECT_3RDPARTY_PATH}/interfaces/org.deepin.ai.daemon.AnalyzeServer.xml analyzeserver)
# else()
#     qt5_add_dbus_interface(IFS_SRC ${PROJECT_3RDPARTY_PATH}/interfaces/org.deepin.ai.daemon.AnalyzeServer.xml analyzeserver)
# endif()

FILE(GLOB 3RD_FSEARCH_SRC
    ${PROJECT_3RDPARTY_PATH}/fsearch/*
    ${PROJECT_3RDPARTY_PATH}/lucene/*
    )
pkg_check_modules(GLIB REQUIRED glib-2.0)
pkg_check_modules(PCRE libpcre)
pkg_check_modules(Lucene REQUIRED IMPORTED_TARGET liblucene++ liblucene++-contrib)

if (NOT (${CMAKE_SYSTEM_PROCESSOR} MATCHES "mips" OR ${CMAKE_SYSTEM_PROCESSOR} MATCHES "aarch64" OR ${CMAKE_SYSTEM_PROCESSOR} MATCHES "sw"))
    # ENABLE_DEEPINANYTHING 已不再需要：anything_interface 已移除
    # ADD_DEFINITIONS(-DENABLE_DEEPINANYTHING)
else()
    # 移除不支持的文件
    FILE(GLOB_RECURSE RM_DASRC
        "${PROJECT_DAEMONSRC_PATH}/searcher/file/filename*"
        "${PROJECT_DAEMONSRC_PATH}/searcher/file/anything*"
        )
endif()

# 项目工程文件
FILE(GLOB_RECURSE PRO_SRC
    "${PROJECT_DAEMONSRC_PATH}/*/*.h"
    "${PROJECT_DAEMONSRC_PATH}/*/*.cpp"
    "${PROJECT_SOURCE_PATH}/grand-search-daemon/daemonlibrary.h"
    "${PROJECT_SOURCE_PATH}/grand-search-daemon/daemonlibrary.cpp"
    )

FILE(GLOB_RECURSE TOOL_SRC
    "${PROJECT_TOOLS_PATH}/*/*.h"
    "${PROJECT_TOOLS_PATH}/*/*.cpp"
    )

# 移除未编写的文件
FILE(GLOB_RECURSE RM_SRC
    "${PROJECT_DAEMONSRC_PATH}/searchplugin/interface/abstract/*"
    )
list(REMOVE_ITEM PRO_SRC ${RM_SRC} ${RM_DASRC})

#单元测试文件
FILE(GLOB_RECURSE UT_SRC "./*/*.cpp")

set(Qt_LIBS
    ${QT_NS}::Core
    ${QT_NS}::Gui
    ${QT_NS}::DBus
    ${QT_NS}::Concurrent
)

set(SRCS
    main.cpp
    ut_commontools.cpp
    ${GLOBAL_SRC}
    ${PRO_SRC}
    ${TOOL_SRC}
    ${UT_SRC}
    ${CPP_STUB_SRC}
    ${3RD_FSEARCH_SRC}
)

# 可执行程序
add_executable(${BIN_NAME} ${SRCS})

target_include_directories(${BIN_NAME} PUBLIC
    ${CMAKE_SOURCE_DIR}/include
    ${PROJECT_3RDPARTY_PATH}/fsearch
    ${GLIB_INCLUDE_DIRS}
    ${PCRE_INCLUDE_DIRS}
    ${Lucene_INCLUDE_DIRS}
)

target_link_libraries(${BIN_NAME} PRIVATE
    ${Qt_LIBS}
    ${DtkCore_LIBRARIES}
    ${GTEST_LIBRARIES}
    ${QT_NS}::Test
    ${GLIB_LIBRARIES}
    ${PCRE_LIBRARIES}
    ${Lucene_LIBRARIES}
    -lpthread
    -lboost_system
)
