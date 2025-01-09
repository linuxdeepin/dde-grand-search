cmake_minimum_required(VERSION 3.10)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)

set(LIB_NAME semanticparser)

find_package(${QT_NS} COMPONENTS Gui DBus REQUIRED)
find_package(PkgConfig REQUIRED)


FILE(GLOB_RECURSE SRC_FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
)

if (QT_VERSION_MAJOR EQUAL 6)
    qt6_add_dbus_interface(IFS_SRC ${PROJECT_3RDPARTY_PATH}/interfaces/org.deepin.ai.daemon.VectorIndex.xml vectorindex)
    qt6_add_dbus_interface(IFS_SRC ${PROJECT_3RDPARTY_PATH}/interfaces/org.deepin.ai.daemon.QueryLang.xml querylang)
else()
    qt5_add_dbus_interface(IFS_SRC ${PROJECT_3RDPARTY_PATH}/interfaces/org.deepin.ai.daemon.VectorIndex.xml vectorindex)
    qt5_add_dbus_interface(IFS_SRC ${PROJECT_3RDPARTY_PATH}/interfaces/org.deepin.ai.daemon.QueryLang.xml querylang)
endif()

add_library(${LIB_NAME} STATIC ${SRC_FILES} ${IFS_SRC})

target_include_directories(${LIB_NAME} PRIVATE
    ${CMAKE_CURRENT_BINARY_DIR}
)

target_link_libraries(
    ${LIB_NAME}
    ${QT_NS}::Gui
)
