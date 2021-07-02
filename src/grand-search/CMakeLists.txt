# 定义需要的cmake版本
cmake_minimum_required(VERSION 3.10)

# 定义可执行程序名称
set(BIN_NAME dde-grand-search)

# 依赖包
find_package(PkgConfig REQUIRED)
find_package(DtkWidget REQUIRED)
find_package(DtkGui REQUIRED)
find_package(DtkCMake REQUIRED)
find_package(Qt5 COMPONENTS
    Core
    Widgets
    Concurrent
    DBus
REQUIRED)

set(Qt_LIBS
    Qt5::Core
    Qt5::Gui
    Qt5::DBus
)
