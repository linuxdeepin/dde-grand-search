set(CMAKE_CXX_STANDARD 17)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")

set(FRONTEND_NAME dde-grand-search)

# 应用库目录
set(LIB_BASE_DIR "${LIB_PLATFORM_DIR}/${FRONTEND_NAME}")

# 插件目录
set(PLUGIN_BASE_DIR "${LIB_BASE_DIR}/plugins")

# 预览插件目录
set(PLUGIN_PREVIEW_DIR "${PLUGIN_BASE_DIR}/preview")
add_definitions(-DPLUGIN_PREVIEW_DIR="${PLUGIN_PREVIEW_DIR}")

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
REQUIRED)

set(Qt_LIBS
    ${QT_NS}::Core
    ${QT_NS}::Gui
    ${QT_NS}::Widgets
    ${QT_NS}::Concurrent
)
set(DTK_LIBS
    ${DTK_NS}::Gui
    ${DTK_NS}::Widget
)


set(QRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/resource.qrc
)

# 前端
set(FRONTEND_NAME dde-grand-search)

include_directories(${CMAKE_SOURCE_DIR}/src/grand-search/gui/exhibition/preview)

add_subdirectory(text-preview)
add_subdirectory(image-preview)
add_subdirectory(audio-preview)
add_subdirectory(pdf-preview)
add_subdirectory(video-preview)
