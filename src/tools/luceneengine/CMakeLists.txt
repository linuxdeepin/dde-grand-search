cmake_minimum_required(VERSION 3.10)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)

set(LIB_NAME luceneengine)

find_package(${QT_NS} COMPONENTS Gui REQUIRED)
find_package(PkgConfig REQUIRED)
find_package(dfm${DTK_VERSION_MAJOR}-search REQUIRED)

pkg_check_modules(Lucene REQUIRED IMPORTED_TARGET liblucene++ liblucene++-contrib)

FILE(GLOB_RECURSE SRC_FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
)

FILE(GLOB_RECURSE LUCENE_FILES
    "${PROJECT_3RDPARTY_PATH}/lucene/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/lucene/*.cpp"
)

add_library(${LIB_NAME} STATIC ${SRC_FILES} ${LUCENE_FILES})

target_include_directories(${LIB_NAME}
    PRIVATE
        ${CMAKE_CURRENT_BINARY_DIR}
)

target_link_libraries(
    ${LIB_NAME}
    ${QT_NS}::Gui
    PkgConfig::Lucene
    dfm${DTK_VERSION_MAJOR}-search
)
