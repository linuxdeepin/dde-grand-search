# 定义需要的cmake版本
cmake_minimum_required(VERSION 3.10)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Wall -Wl,--as-need -fPIE")

# QT_DEBUG: pluginmanager.cpp readConf() uses #ifdef QT_DEBUG to avoid PLUGIN_SEARCHER_DIR
add_definitions(-DQT_DEBUG)

# 定义可执行程序名称
set(BIN_NAME test-libdde-grand-search-daemon)

# 依赖包
find_package(PkgConfig REQUIRED)
find_package(${DTK_CORE_PKG} REQUIRED)
find_package(${QT_NS} COMPONENTS
    Core
    Gui
    DBus
    Concurrent
REQUIRED)

# 源码路径 — dde-grand-search-daemon（非 libgrand-search-daemon）
set(PROJECT_DAEMONSRC_PATH "${PROJECT_SOURCE_PATH}/dde-grand-search-daemon")
include_directories(${PROJECT_DAEMONSRC_PATH})

# ── utils 模块源码 ──
set(UTILS_SRC
    "${PROJECT_DAEMONSRC_PATH}/utils/specialtools.cpp"
    "${PROJECT_DAEMONSRC_PATH}/utils/chineseletterhelper.cpp"
)

# utils 单元测试文件
set(UTILS_UT_SRC
    "utils/ut_specialtools.cpp"
    "utils/ut_chineseletterhelper.cpp"
)

# ── searchplugin 模块源码 ──
set(SEARCHPLUGIN_SRC
    "${PROJECT_DAEMONSRC_PATH}/searchplugin/convertors/abstractconvertor.cpp"
    "${PROJECT_DAEMONSRC_PATH}/searchplugin/convertors/convertorv1_0.cpp"
    "${PROJECT_DAEMONSRC_PATH}/searchplugin/convertors/dataconvertor.cpp"
    "${PROJECT_DAEMONSRC_PATH}/searchplugin/interface/searchplugininterfacev1.cpp"
    "${PROJECT_DAEMONSRC_PATH}/searchplugin/loader/pluginloader.cpp"
    "${PROJECT_DAEMONSRC_PATH}/searchplugin/pluginliaison.cpp"
    "${PROJECT_DAEMONSRC_PATH}/searchplugin/pluginmanager.cpp"
    "${PROJECT_DAEMONSRC_PATH}/searchplugin/process/pluginprocess.cpp"
)

# searchplugin 单元测试文件
set(SEARCHPLUGIN_UT_SRC
    "searchplugin/ut_abstractconvertor.cpp"
    "searchplugin/ut_convertorv1_0.cpp"
    "searchplugin/ut_dataconvertor.cpp"
    "searchplugin/ut_pluginliaison.cpp"
    "searchplugin/ut_pluginloader.cpp"
    "searchplugin/ut_pluginmanager.cpp"
    "searchplugin/ut_pluginprocess.cpp"
    "searchplugin/ut_searchplugininterfacev1.cpp"
)

# logDaemon 定义文件（源码中 Q_DECLARE_LOGGING_CATEGORY，定义在 main.cpp 之外单独提供）
set(LOGDAEMON_SRC "${CMAKE_CURRENT_BINARY_DIR}/logdaemon_def.cpp")
file(WRITE ${LOGDAEMON_SRC}
    "#include <QLoggingCategory>\n"
    "Q_LOGGING_CATEGORY(logDaemon, \"org.deepin.dde.GrandSearch.Daemon\")\n"
)

# 资源文件 — pinyin.dict
set(UTILS_QRC "${PROJECT_DAEMONSRC_PATH}/utils/dict.qrc")

set(SRCS
    main.cpp
    ${LOGDAEMON_SRC}
    ${UTILS_SRC}
    ${SEARCHPLUGIN_SRC}
    ${UTILS_UT_SRC}
    ${SEARCHPLUGIN_UT_SRC}
    ${UTILS_QRC}
    ${CPP_STUB_SRC}
)

set(Qt_LIBS
    ${QT_NS}::Core
    ${QT_NS}::Gui
    ${QT_NS}::DBus
    ${QT_NS}::Concurrent
)

# 可执行程序
add_executable(${BIN_NAME} ${SRCS})

target_include_directories(${BIN_NAME} PUBLIC
    ${CMAKE_SOURCE_DIR}/include
    ${PROJECT_3RDPARTY_PATH}
)

target_link_libraries(${BIN_NAME} PRIVATE
    ${Qt_LIBS}
    ${DtkCore_LIBRARIES}
    ${GTEST_LIBRARIES}
    ${QT_NS}::Test
    -lpthread
)
