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
    business/query/querycontroller_p.h
    business/query/querycontroller.h
    business/query/querycontroller.cpp
    business/matchresult/matchcontroller_p.h
    business/matchresult/matchcontroller.h
    business/matchresult/matchcontroller.cpp
    )
# 源文件
set(SRCS
    main.cpp
)

add_executable(${BIN_NAME} ${SRCS} ${GUISRC} ${BUSINESSSRC})
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

# bin
install(TARGETS ${BIN_NAME} DESTINATION bin)
