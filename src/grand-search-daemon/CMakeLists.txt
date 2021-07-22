# 定义需要的cmake版本
cmake_minimum_required(VERSION 3.10)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Wall -Wl,--as-need -fPIE")

# 定义可执行程序名称
set(BIN_NAME dde-grand-search-daemon)

# 应用库目录
set(LIB_BASE_DIR "${LIB_PLATFORM_DIR}/${BIN_NAME}")

# 插件目录
set(PLUGIN_DIR "${LIB_BASE_DIR}/plugins")

# 扩展搜索插件目录
set(PLUGIN_SEARCHER_DIR "${PLUGIN_DIR}/searcher")
add_definitions(-DPLUGIN_SEARCHER_DIR="${PLUGIN_SEARCHER_DIR}")

# 依赖包
#find_package(PkgConfig REQUIRED)
find_package(DtkCore REQUIRED)
#find_package(DtkCMake REQUIRED)
find_package(Qt5 COMPONENTS
    Core
    Gui
    Concurrent
    DBus
REQUIRED)

set(Qt_LIBS
    Qt5::Core
    Qt5::Gui
    Qt5::DBus
    Qt5::Concurrent
)

# 源文件
set(DBUS
    dbusservice/grandsearchinterfaceprivate.h
    dbusservice/grandsearchinterface.h
    dbusservice/grandsearchinterface.cpp
)

set(MAINCTRL
    maincontroller/maincontroller.h
    maincontroller/maincontrollerprivate.h
    maincontroller/maincontroller.cpp
    maincontroller/task/taskcommander.h
    maincontroller/task/taskcommander.cpp
    maincontroller/task/taskcommanderprivate.h
)

set(SEARCHER
    searcher/searchergroup.h
    searcher/searchergroupprivate.h
    searcher/searchergroup.cpp
    searcher/searcher.h
    searcher/searcher.cpp
    searcher/proxyworker.h
    searcher/proxyworker.cpp
    searcher/app/desktopappsearcher.h
    searcher/app/desktopappsearcherprivate.h
    searcher/app/desktopappsearcher.cpp
    searcher/app/desktopappworker.h
    searcher/app/desktopappworker.cpp
#    searcher/setting/controlcentersearcher.h
#    searcher/setting/controlcentersearcher.cpp
#    searcher/fulltext/fulltextsearcher.h
#    searcher/fulltext/fulltextsearcher.cpp
    searcher/extend/extendsearcher.h
    searcher/extend/extendsearcherprivate.h
    searcher/extend/extendsearcher.cpp
)
#后期需要添加到使用anything的情况下
qt5_add_dbus_interface(ANYTHING_SRC /usr/share/dbus-1/interfaces/com.deepin.anything.xml anything_interface)

#未实现，编译不过
if (0)
#文件搜索
if (${CMAKE_SYSTEM_PROCESSOR} MATCHES "mips" OR ${CMAKE_SYSTEM_PROCESSOR} MATCHES "arm")
    message(${CMAKE_SYSTEM_PROCESSOR} ": using fsearch")
    ADD_DEFINITIONS(-DENABLE_FSEARCH)
    set(SEARCHER
        ${SEARCHER}
        searcher/file/fssearcher.h
        searcher/file/fssearcher.cpp
        searcher/file/fsworker.h
        searcher/file/fsworker.cpp
    )
else ()
    message(${CMAKE_SYSTEM_PROCESSOR} ": using deepin-anything.")
    ADD_DEFINITIONS(-DENABLE_DEEPINANYTHING)
    set(SEARCHER
        ${SEARCHER}
        ${ANYTHING_SRC}
        searcher/file/filenamesearcher.h
        searcher/file/filenamesearcher.cpp
        searcher/file/filenameworker.h
        searcher/file/filenameworker.cpp
        searcher/file/filenameworkerprivate.h
    )
endif()
#测试，发布前需删除以下内容
ADD_DEFINITIONS(-DENABLE_FSEARCH)
set(SEARCHER
    ${SEARCHER}
    searcher/file/fssearcher.h
    searcher/file/fssearcher.cpp
    searcher/file/fsworker.h
    searcher/file/fsworker.cpp
)
endif()

ADD_DEFINITIONS(-DENABLE_DEEPINANYTHING)
set(SEARCHER
    ${SEARCHER}
    ${ANYTHING_SRC}
    searcher/file/filenamesearcher.h
    searcher/file/filenamesearcher.cpp
    searcher/file/filenameworker.h
    searcher/file/filenameworker.cpp
    searcher/file/filenameworkerprivate.h
)
message("local test fsearch and deepin-anything")
#end

set(SEARCHPLUGIN
    searchplugin/pluginmanager.h
    searchplugin/pluginmanagerprivate.h
    searchplugin/pluginmanager.cpp
    searchplugin/loader/pluginloader.h
    searchplugin/loader/pluginloader.cpp
    searchplugin/process/pluginprocess.h
    searchplugin/process/pluginprocess.cpp
    searchplugin/dataconvertor.h
    searchplugin/dataconvertor.cpp
    searchplugin/convertors/abstractconvertor.h
)

set(UTILS
    utils/searchpluginconf.h
    utils/searchplugininfo.h
    utils/utiltools.h
    utils/utiltools.cpp
)

set(SRCS
    main.cpp
    ${GLOBAL_SRC}
    ${UTILS}
    ${DBUS}
    ${MAINCTRL}
    ${SEARCHER}
    ${SEARCHPLUGIN}
)

# 可执行程序
add_executable(${BIN_NAME} ${SRCS})

target_include_directories(${BIN_NAME} PUBLIC
    ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(${BIN_NAME} PRIVATE
    ${Qt_LIBS}
    ${DtkCore_LIBRARIES}
)

# bin
install(TARGETS ${BIN_NAME} DESTINATION bin)

