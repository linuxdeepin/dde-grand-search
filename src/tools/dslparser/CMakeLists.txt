cmake_minimum_required(VERSION 3.10)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)

set(LIB_NAME dslparser)

find_package(Qt5 COMPONENTS Gui DBus REQUIRED)
find_package(PkgConfig REQUIRED)


FILE(GLOB_RECURSE SRC_FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
)
set(QUERYLANG_SRC
    ${PROJECT_3RDPARTY_PATH}/antlr4/querylang/querylangLexer.cpp
    ${PROJECT_3RDPARTY_PATH}/antlr4/querylang/querylangParser.cpp
    ${PROJECT_3RDPARTY_PATH}/antlr4/querylang/querylangBaseListener.cpp
    ${PROJECT_3RDPARTY_PATH}/antlr4/querylang/querylangListener.cpp
)

qt5_add_dbus_interface(IFS_SRC ${PROJECT_3RDPARTY_PATH}/interfaces/org.deepin.ai.daemon.AnalyzeServer.xml analyzeserver)
qt5_add_dbus_interface(IFS_SRC ${PROJECT_3RDPARTY_PATH}/interfaces/org.deepin.ai.daemon.VectorIndex.xml vectorindex)

add_library(${LIB_NAME} STATIC
    ${SRC_FILES}
    ${QUERYLANG_SRC}
    ${IFS_SRC}
)

target_include_directories(${LIB_NAME} PRIVATE
    ${CMAKE_CURRENT_BINARY_DIR}
    ${PROJECT_3RDPARTY_PATH}/antlr4/querylang
    ${PROJECT_3RDPARTY_PATH}/antlr4/antlr4-runtime/runtime/src
)

target_link_libraries(${LIB_NAME}
    Qt5::Gui
    antlr4_static
)
