# 定义需要的cmake版本
cmake_minimum_required(VERSION 3.10)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIE -pie")

# 设置名称
set(BIN_NAME ${DAEMON_BIN_NAME})

# 依赖包
find_package(PkgConfig REQUIRED)
find_package(${DTK_NS} COMPONENTS
    Gui
    Widget
REQUIRED)
find_package(${QT_NS} COMPONENTS
    Core
    Gui
    Widgets
    Concurrent
    DBus
REQUIRED)
find_package(deepin-qdbus-service REQUIRED)
find_package(dfm${DTK_VERSION_MAJOR}-search REQUIRED)

add_definitions(-DSERVICE_CONFIG_DIR="${CMAKE_INSTALL_PREFIX}/share/deepin-service-manager/")

set(Qt_LIBS
    ${QT_NS}::Core
    ${QT_NS}::Gui
    ${QT_NS}::Widgets
    ${QT_NS}::DBus
    ${QT_NS}::Concurrent
)

set(DTK_LIBS
    ${DTK_NS}::Gui
    ${DTK_NS}::Widget
)

# 依赖库
set(LINK_LIBS
    ${Qt_LIBS}
    ${DTK_LIBS}
    ${deepin-qdbus-service_LIBRARIES}
    dfm${DTK_VERSION_MAJOR}-search
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

FILE(GLOB_RECURSE SEMANTIC_FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/searcher/semantic/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/searcher/semantic/*.cpp"
    "${PROJECT_3RDPARTY_PATH}/antlr4/querylang/*.h"
    "${PROJECT_3RDPARTY_PATH}/antlr4/querylang/*.cpp"
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
    searcher/web/statictextechoer.h
    searcher/web/statictextechoer.cpp
    searcher/web/statictextworker.h
    searcher/web/statictextworker.cpp
    searcher/extend/extendsearcher.h
    searcher/extend/extendsearcher_p.h
    searcher/extend/extendsearcher.cpp
    searcher/extend/extendworker.h
    searcher/extend/extendworker.cpp
    searcher/file/filenamesearcher.h
    searcher/file/filenamesearcher.cpp
    searcher/file/filenameworker.h
    searcher/file/filenameworker.cpp
    searcher/file/filenameworker_p.h
    searcher/file/filesearchutils.h
    searcher/file/filesearchutils.cpp
)

if (${CMAKE_SYSTEM_PROCESSOR} MATCHES "x86_64")
    ADD_DEFINITIONS(-DENABLE_SEMANTIC)
    set(SEARCHER
        ${SEARCHER}
        ${SEMANTIC_FILES}
    )

    set(LINK_LIBS
        ${LINK_LIBS}
        semanticparser
        luceneengine
        antlr4_static
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

set(GDCONFIGURATION
    configuration/configer.h
    configuration/configer_p.h
    configuration/configer.cpp
    configuration/preferenceitem.h
    configuration/userpreference.h
    configuration/userpreference.cpp
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
    ${GDCONFIGURATION}
)

if (${QT_VERSION_MAJOR} EQUAL 5)
    qt5_add_resources(QRC_FILES utils/dict.qrc)
else()
    qt_add_resources(QRC_FILES utils/dict.qrc)
endif()

# 执行程序
add_executable(${BIN_NAME}
    ${SRCS} ${QRC_FILES}
)

target_include_directories(${BIN_NAME} PUBLIC
    ${CMAKE_SOURCE_DIR}/include
    ${GLIB_INCLUDE_DIRS}
    ${PCRE_INCLUDE_DIRS}
    ${deepin-qdbus-service_INCLUDE_DIR}
    ${PROJECT_3RDPARTY_PATH}/antlr4/querylang
    ${PROJECT_3RDPARTY_PATH}/antlr4/antlr4-runtime/runtime/src
)

target_link_libraries(${BIN_NAME} PRIVATE
    ${LINK_LIBS}
)

install(TARGETS ${BIN_NAME} DESTINATION ${CMAKE_INSTALL_BINDIR})
install(FILES grand-search-daemon.json DESTINATION share/deepin-service-manager/other/)
install(FILES org.deepin.dde.GrandSearchDaemon.service DESTINATION ${CMAKE_INSTALL_FULL_DATADIR}/dbus-1/services)

# 扩展搜索插件目录
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/.readme DESTINATION ${PLUGIN_SEARCHER_DIR})
