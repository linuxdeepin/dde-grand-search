# 定义需要的cmake版本
cmake_minimum_required(VERSION 3.10)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Wall -Wl,--as-need -fPIE")

# 定义可执行程序名称
set(BIN_NAME ut-dde-grand-search-daemon)

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

set(PROJECT_DAEMONSRC_PATH "${PROJECT_SOURCE_PATH}/grand-search-daemon")
include_directories(${PROJECT_DAEMONSRC_PATH})

# 项目工程文件
FILE(GLOB_RECURSE PRO_SRC "${PROJECT_DAEMONSRC_PATH}/*/*.h" "${PROJECT_DAEMONSRC_PATH}/*/*.cpp")

# 移除未编写的文件
FILE(GLOB_RECURSE RM_SRC
    "${PROJECT_DAEMONSRC_PATH}/searcher/extend/*"
    "${PROJECT_DAEMONSRC_PATH}/searcher/file/*"
    "${PROJECT_DAEMONSRC_PATH}/searcher/fulltext/*"
    "${PROJECT_DAEMONSRC_PATH}/searcher/setting/*"
    )
list(REMOVE_ITEM PRO_SRC ${RM_SRC})

#单元测试文件
FILE(GLOB_RECURSE UT_SRC "./*/*.cpp")

set(Qt_LIBS
    Qt5::Core
    Qt5::Gui
    Qt5::DBus
    Qt5::Concurrent
)

set(SRCS
    main.cpp
    ${GLOBAL_SRC}
    ${PRO_SRC}
    ${UT_SRC}
    ${CPP_STUB_SRC}
)

# 可执行程序
add_executable(${BIN_NAME} ${SRCS})

target_include_directories(${BIN_NAME} PUBLIC
    ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(${BIN_NAME} PRIVATE
    ${Qt_LIBS}
    ${DtkCore_LIBRARIES}
    ${GTEST_LIBRARIES}
    ${Qt5Test_LIBRARIES}
    -lpthread
)
