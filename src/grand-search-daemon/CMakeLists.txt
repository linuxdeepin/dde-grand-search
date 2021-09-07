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
find_package(PkgConfig REQUIRED)
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

# 依赖库
set(LINK_LIBS
    ${Qt_LIBS}
    ${DtkCore_LIBRARIES}
)

# 源文件
set(DBUS
    dbusservice/grandsearchinterface_p.h
    dbusservice/grandsearchinterface.h
    dbusservice/grandsearchinterface.cpp
)

set(MAINCTRL
    maincontroller/maincontroller.h
    maincontroller/maincontroller_p.h
    maincontroller/maincontroller.cpp
    maincontroller/task/taskcommander.h
    maincontroller/task/taskcommander.cpp
    maincontroller/task/taskcommander_p.h
)

set(SEARCHER
    searcher/searchergroup.h
    searcher/searchergroup_p.h
    searcher/searchergroup.cpp
    searcher/searcher.h
    searcher/searcher.cpp
    searcher/proxyworker.h
    searcher/proxyworker.cpp
    searcher/app/desktopappsearcher.h
    searcher/app/desktopappsearcher_p.h
    searcher/app/desktopappsearcher.cpp
    searcher/app/desktopappworker.h
    searcher/app/desktopappworker.cpp
    searcher/setting/controlcentersearcher.h
    searcher/setting/controlcentersearcherprivate.h
    searcher/setting/controlcentersearcher.cpp
    searcher/setting/controlcenterworker.h
    searcher/setting/controlcenterworker.cpp
    searcher/web/statictextechoer.h
    searcher/web/statictextechoer.cpp
    searcher/web/statictextworker.h
    searcher/web/statictextworker.cpp
    searcher/extend/extendsearcher.h
    searcher/extend/extendsearcher_p.h
    searcher/extend/extendsearcher.cpp
    searcher/extend/extendworker.h
    searcher/extend/extendworker.cpp
)

#文件搜索
if (${CMAKE_SYSTEM_PROCESSOR} MATCHES "mips" OR ${CMAKE_SYSTEM_PROCESSOR} MATCHES "aarch64" OR ${CMAKE_SYSTEM_PROCESSOR} MATCHES "sw")
    message(${CMAKE_SYSTEM_PROCESSOR} ": using fsearch")
    ADD_DEFINITIONS(-DENABLE_FSEARCH)
    FILE(GLOB 3RD_FSEARCH_SRC ${CMAKE_SOURCE_DIR}/3rdparty/fsearch/*)
    pkg_check_modules(GLIB REQUIRED glib-2.0)
    set(SEARCHER
        ${SEARCHER}
        ${3RD_FSEARCH_SRC}
        searcher/file/fssearcher.h
        searcher/file/fssearcher.cpp
        searcher/file/fsworker.h
        searcher/file/fsworker.cpp
    )
    set(LINK_LIBS
        ${LINK_LIBS}
        ${GLIB_LIBRARIES}
    )
else ()
    message(${CMAKE_SYSTEM_PROCESSOR} ": using deepin-anything.")
    ADD_DEFINITIONS(-DENABLE_DEEPINANYTHING)
    qt5_add_dbus_interface(ANYTHING_SRC /usr/share/dbus-1/interfaces/com.deepin.anything.xml anything_interface)
    set(SEARCHER
        ${SEARCHER}
        ${ANYTHING_SRC}
        searcher/file/filenamesearcher.h
        searcher/file/filenamesearcher.cpp
        searcher/file/filenameworker.h
        searcher/file/filenameworker.cpp
        searcher/file/filenameworker_p.h
    )
endif()

# 手动生成xml描述文件
# qdbuscpp2xml -A grandsearchplugininterfacev1.h -o com.deepin.dde.grandsearch.searchplugin.xml
set(PLUGIN_ABSTRACT_IFS
    searchplugin/interface/abstract/grandsearchplugininterfacev1.h
)

# 插件通信接口
set(SEARCHPLUGIN_IFS
    searchplugin/interface/searchplugininterfacev1.h
    searchplugin/interface/searchplugininterfacev1.cpp
)

# 插件通信协议
set(SEARCHPLUGIN_PROT
    searchplugin/convertors/abstractconvertor.h
    searchplugin/convertors/abstractconvertor.cpp
    searchplugin/convertors/convertorv1_0.h
    searchplugin/convertors/convertorv1_0.cpp
)

set(SEARCHPLUGIN
    searchplugin/pluginmanager.h
    searchplugin/pluginmanager_p.h
    searchplugin/pluginmanager.cpp
    searchplugin/loader/pluginloader.h
    searchplugin/loader/pluginloader.cpp
    searchplugin/process/pluginprocess.h
    searchplugin/process/pluginprocess.cpp
    searchplugin/convertors/dataconvertor.h
    searchplugin/convertors/dataconvertor.cpp
    searchplugin/pluginliaison.h
    searchplugin/pluginliaison_p.h
    searchplugin/pluginliaison.cpp
    ${SEARCHPLUGIN_IFS}
    ${SEARCHPLUGIN_PROT}
)

set(UTILS
    utils/searchpluginconf.h
    utils/searchplugininfo.h
    utils/searchpluginprotocol.h
    utils/specialtools.h
    utils/specialtools.cpp
    utils/chineseletterhelper.h
    utils/chineseletterhelper.cpp
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

# 资源文件
qt5_add_resources(QRC_FILES utils/dict.qrc)

# 可执行程序
add_executable(${BIN_NAME} ${SRCS} ${QRC_FILES})

target_include_directories(${BIN_NAME} PUBLIC
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/3rdparty/fsearch
    ${GLIB_INCLUDE_DIRS}
)

target_link_libraries(${BIN_NAME} PRIVATE
    ${LINK_LIBS}
)

# bin
install(TARGETS ${BIN_NAME} DESTINATION bin)

# dbus
install(FILES data/com.deepin.dde.daemon.GrandSearch.service DESTINATION share/dbus-1/services)

# autostart
install(FILES data/dde-grand-search-daemon.desktop DESTINATION /etc/xdg/autostart)
