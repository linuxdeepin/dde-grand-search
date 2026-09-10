# 定义需要的cmake版本
cmake_minimum_required(VERSION 3.10)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Wall -Wl,--as-need -fPIE")

# 定义可执行程序名称
set(BIN_NAME test-libdde-grand-search-daemon)

# 依赖包
find_package(PkgConfig REQUIRED)
find_package(${DTK_CORE_PKG} REQUIRED)
find_package(${QT_NS} COMPONENTS
    Core
    Gui
REQUIRED)

# 源码路径 — dde-grand-search-daemon（非 libgrand-search-daemon）
set(PROJECT_DAEMONSRC_PATH "${PROJECT_SOURCE_PATH}/dde-grand-search-daemon")
include_directories(${PROJECT_DAEMONSRC_PATH})

# 仅编译 utils 模块源码（测试范围限定为 utils/）
set(UTILS_SRC
    "${PROJECT_DAEMONSRC_PATH}/utils/specialtools.cpp"
    "${PROJECT_DAEMONSRC_PATH}/utils/chineseletterhelper.cpp"
)

# 单元测试文件 — 仅 utils 子目录中属于本测试范围的文件
set(UTILS_UT_SRC
    "utils/ut_specialtools.cpp"
    "utils/ut_chineseletterhelper.cpp"
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
    ${UTILS_UT_SRC}
    ${UTILS_QRC}
    ${CPP_STUB_SRC}
)

set(Qt_LIBS
    ${QT_NS}::Core
    ${QT_NS}::Gui
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
