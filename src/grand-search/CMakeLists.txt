# 定义需要的cmake版本
cmake_minimum_required(VERSION 3.10)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_FLAGS "-g -Wall -Wl,--as-need -fPIE")

# 定义可执行程序名称
set(BIN_NAME dde-grand-search)

# 依赖包
find_package(PkgConfig REQUIRED)
find_package(DtkWidget REQUIRED)
find_package(DtkGui REQUIRED)
find_package(DtkCMake REQUIRED)
find_package(Qt5 COMPONENTS
    Core
    Gui
    Widgets
    Concurrent
    DBus
REQUIRED)

set(Qt_LIBS
    Qt5::Core
    Qt5::Gui
    Qt5::Widgets
    Qt5::DBus
)

# gui
set(GUISRC
    gui/mainwindow_p.h
    gui/mainwindow.h
    gui/mainwindow.cpp
    # 入口界面
    gui/entrance/entrancewidget_p.h
    gui/entrance/entrancewidget.h
    gui/entrance/entrancewidget.cpp
    # 展示界面
    gui/exhibition/exhibitionwidget_p.h
    gui/exhibition/exhibitionwidget.h
    gui/exhibition/exhibitionwidget.cpp
    gui/exhibition/matchresult/matchwidget_p.h
    gui/exhibition/matchresult/matchwidget.h
    gui/exhibition/matchresult/matchwidget.cpp
    )

# business
set(BUSINESSSRC
    # 查询控制
    business/query/querycontroller_p.h
    business/query/querycontroller.h
    business/query/querycontroller.cpp
    # 匹配结果控制
    business/matchresult/matchcontroller_p.h
    business/matchresult/matchcontroller.h
    business/matchresult/matchcontroller.cpp
    )

# contacts
set(CONTACTSSRC
    # dbus服务
    contacts/services/grandsearchservice_p.h
    contacts/services/grandsearchservice.h
    contacts/services/grandsearchservice.cpp
    contacts/services/grandsearchserviceadaptor.h
    contacts/services/grandsearchserviceadaptor.cpp
    )

# 源文件
set(SRCS
    main.cpp
)

add_executable(${BIN_NAME} ${SRCS} ${GUISRC} ${BUSINESSSRC} ${CONTACTSSRC})
target_include_directories(${BIN_NAME} PUBLIC
    ${CMAKE_SOURCE_DIR}/include
    ${DtkWidget_INCLUDE_DIRS}
    ${DtkGUI_INCLUDE_DIRS}
)

target_link_libraries(${BIN_NAME} PRIVATE
    ${Qt_LIBS}
    ${DtkWidget_LIBRARIES}
    ${DtkGUI_LIBRARIES}
)

# Install settings
if (CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(CMAKE_INSTALL_PREFIX /usr)
endif ()

# bin
install(TARGETS ${BIN_NAME} DESTINATION bin)

# dbus
install(FILES contacts/services/com.deepin.dde.GrandSearch.service DESTINATION share/dbus-1/services)
install(FILES contacts/services/com.deepin.dde.GrandSearch.xml DESTINATION share/dbus-1/interfaces)
