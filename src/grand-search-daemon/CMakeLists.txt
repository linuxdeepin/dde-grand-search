# 定义需要的cmake版本
cmake_minimum_required(VERSION 3.10)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_FLAGS "-g -Wall -Wl,--as-need -fPIE")

# 定义可执行程序名称
set(BIN_NAME dde-grand-search-daemon)

# 依赖包
find_package(PkgConfig REQUIRED)
find_package(DtkWidget REQUIRED)
find_package(DtkGui REQUIRED)
find_package(DtkCMake REQUIRED)
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
)

set(SEARCHER
    searcher/searchergroup.h
    searcher/searchergroup.cpp
    searcher/searcher.h
    searcher/searcher.cpp
    searcher/proxyworker.h
    searcher/proxyworker.cpp
    searcher/file/filenamesearcher.h
    searcher/file/filenamesearcher.cpp
    searcher/app/desktopappsearcher.h
    searcher/app/desktopappsearcher.cpp
#    searcher/setting/controlcentersearcher.h
#    searcher/setting/controlcentersearcher.cpp
#    searcher/fulltext/fulltextsearcher.h
#    searcher/fulltext/fulltextsearcher.cpp
    searcher/extend/extendsearcher.h
    searcher/extend/extendsearcher.cpp
)

set(SEARCHPLUGIN
    searchplugin/pluginmanager.h
    searchplugin/pluginmanager.cpp
    searchplugin/loader/pluginloader.h
    searchplugin/loader/pluginloader.cpp
    searchplugin/process/pluginprocess.h
    searchplugin/process/pluginprocess.cpp
    searchplugin/dataconvertor.h
    searchplugin/dataconvertor.cpp
    searchplugin/convertors/abstractconvertor.h
)

set(SRCS
    main.cpp
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
)

# bin
install(TARGETS ${BIN_NAME} DESTINATION bin)
