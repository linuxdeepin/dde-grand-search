# 定义需要的cmake版本
cmake_minimum_required(VERSION 3.10)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIE -pie")

# 定义可执行程序名称
set(BIN_NAME ${SEARCH_BIN_NAME})

# 集成测试标签
ADD_DEFINITIONS(-DENABLE_ACCESSIBILITY)

if (${CMAKE_SYSTEM_PROCESSOR} MATCHES "mips"
        OR ${CMAKE_SYSTEM_PROCESSOR} MATCHES "aarch64"
        OR ${CMAKE_SYSTEM_PROCESSOR} MATCHES "sw"
        OR ${CMAKE_SYSTEM_PROCESSOR} MATCHES "loongarch64"
        )
    message("${CMAKE_SYSTEM_PROCESSOR} : disable ai search")
else()
    message("${CMAKE_SYSTEM_PROCESSOR} : enable ai search")
    ADD_DEFINITIONS(-DENABLE_AI_SEARCH)
endif()

# 依赖包
find_package(PkgConfig REQUIRED)
find_package(${DTK_NS} COMPONENTS
    Core
    Gui
    Widget
REQUIRED)
find_package(${QT_NS} COMPONENTS
    Core
    Gui
    Widgets
    Concurrent
    DBus
    Network
REQUIRED)

set(DBUSSRC
    # dbus接口
    ${PROJECT_3RDPARTY_PATH}/interfaces/daemongrandsearchinterface.h
    ${PROJECT_3RDPARTY_PATH}/interfaces/daemongrandsearchinterface.cpp
    # dbus服务
    ${PROJECT_3RDPARTY_PATH}/services/grandsearchserviceadaptor.h
    ${PROJECT_3RDPARTY_PATH}/services/grandsearchserviceadaptor.cpp
)

set(Qt_LIBS
    ${QT_NS}::Core
    ${QT_NS}::Gui
    ${QT_NS}::Widgets
    ${QT_NS}::DBus
    ${QT_NS}::Concurrent
    ${QT_NS}::Network
)
set(DTK_LIBS
    ${DTK_NS}::Core
    ${DTK_NS}::Gui
    ${DTK_NS}::Widget
)

# gio 打开功能需要依赖的库
pkg_check_modules(GIO_LIB REQUIRED
        gio-unix-2.0
        glib-2.0
)

# gui
set(GUISRC
    gui/mainwindow_p.h
    gui/mainwindow.h
    gui/mainwindow.cpp
    # 可见性/退出处理
    gui/handlevisibility/handlevisibility.h
    gui/handlevisibility/handlevisibility.cpp
    # 界面相关数据定义
    gui/datadefine.h
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
    gui/exhibition/matchresult/groupwidget_p.h
    gui/exhibition/matchresult/groupwidget.h
    gui/exhibition/matchresult/groupwidget.cpp
    # listView
    gui/exhibition/matchresult/listview/grandsearchlistdelegate.h
    gui/exhibition/matchresult/listview/grandsearchlistdelegate.cpp
    gui/exhibition/matchresult/listview/grandsearchlistmodel.h
    gui/exhibition/matchresult/listview/grandsearchlistmodel.cpp
    gui/exhibition/matchresult/listview/grandsearchlistview.h
    gui/exhibition/matchresult/listview/grandsearchlistview.cpp
    # viewMore
    gui/exhibition/matchresult/viewmore/viewmorebutton.cpp
    gui/exhibition/matchresult/viewmore/viewmorebutton.h
    # preview
    gui/exhibition/preview/previewwidget.h
    gui/exhibition/preview/previewwidget.cpp
    gui/exhibition/preview/previewplugin.h
    gui/exhibition/preview/previewplugininterface.h
    gui/exhibition/preview/previewpluginmanager.h
    gui/exhibition/preview/previewpluginmanager.cpp
    gui/exhibition/preview/generalpreviewplugin_p.h
    gui/exhibition/preview/generalpreviewplugin.h
    gui/exhibition/preview/generalpreviewplugin.cpp
    gui/exhibition/preview/generalwidget/generaltoolbar.h
    gui/exhibition/preview/generalwidget/generaltoolbar.cpp
    gui/exhibition/preview/generalwidget/replicablelabel.h
    gui/exhibition/preview/generalwidget/replicablelabel.cpp
    gui/exhibition/preview/generalwidget/detailitem.h
    gui/exhibition/preview/generalwidget/detailitem.cpp
    gui/exhibition/preview/generalwidget/detailwidget.h
    gui/exhibition/preview/generalwidget/detailwidget.cpp
    gui/exhibition/preview/generalwidget/transbutton.h
    gui/exhibition/preview/generalwidget/transbutton.cpp
    gui/exhibition/preview/generalwidget/linkbutton.h
    gui/exhibition/preview/generalwidget/linkbutton.cpp
    gui/exhibition/preview/generalwidget/omitbutton.h
    gui/exhibition/preview/generalwidget/omitbutton.cpp
    gui/exhibition/preview/generalwidget/aitoolbar.h
    gui/exhibition/preview/generalwidget/aitoolbar.cpp
    gui/exhibition/preview/pluginproxy.h
    gui/exhibition/preview/pluginproxy.cpp
    gui/exhibition/preview/previewproxyinterface.h
    # setting
    gui/searchconfig/configwidget.cpp
    gui/searchconfig/configwidget.h
    gui/searchconfig/scopewidget.cpp
    gui/searchconfig/scopewidget.h
    gui/searchconfig/planwidget.cpp
    gui/searchconfig/planwidget.h
    gui/searchconfig/customwidget.cpp
    gui/searchconfig/customwidget.h
    gui/searchconfig/tailerwidget.cpp
    gui/searchconfig/tailerwidget.h
    gui/searchconfig/searchenginewidget.cpp
    gui/searchconfig/searchenginewidget.h
    gui/searchconfig/checkboxwidget/checkboxwidget.cpp
    gui/searchconfig/checkboxwidget/checkboxwidget.h
    gui/searchconfig/checkboxwidget/checkboxitem.cpp
    gui/searchconfig/checkboxwidget/checkboxitem.h
    gui/searchconfig/switchwidget/switchwidget.cpp
    gui/searchconfig/switchwidget/switchwidget.h
    gui/searchconfig/llmwidget/llmwidget.cpp
    gui/searchconfig/llmwidget/llmwidget.h
    gui/searchconfig/llmwidget/modelmanagebutton.cpp
    gui/searchconfig/llmwidget/modelmanagebutton.h
    gui/searchconfig/llmwidget/downloader.h
    gui/searchconfig/llmwidget/downloader.cpp
    gui/searchconfig/llmwidget/embeddingpluginwidget.cpp
    gui/searchconfig/llmwidget/embeddingpluginwidget.h
    gui/searchconfig/hyperlinklabel/hyperlinklabel.cpp
    gui/searchconfig/hyperlinklabel/hyperlinklabel.h
    gui/searchconfig/comboboxwidget/comboboxwidget.h
    gui/searchconfig/comboboxwidget/comboboxwidget.cpp
    gui/searchconfig/bestmatchwidget.cpp
    gui/searchconfig/bestmatchwidget.h
    gui/searchconfig/blacklistview/blacklistview.h
    gui/searchconfig/blacklistview/blacklistview.cpp
    gui/searchconfig/blacklistview/blacklistmodel.cpp
    gui/searchconfig/blacklistview/blacklistmodel.h
    gui/searchconfig/blacklistview/blacklistdelegate.cpp
    gui/searchconfig/blacklistview/blacklistdelegate.h
    gui/searchconfig/blacklistview/deletedialog.cpp
    gui/searchconfig/blacklistview/deletedialog.h
    gui/searchconfig/blacklistwidget.cpp
    gui/searchconfig/blacklistwidget.h
    gui/searchconfig/indexwidget.cpp
    gui/searchconfig/indexwidget.h
    gui/searchconfig/intelligentretrieval/intelligentretrievalwidget.h
    gui/searchconfig/intelligentretrieval/intelligentretrievalwidget.cpp
    gui/searchconfig/roundedbackground.h
    gui/searchconfig/roundedbackground.cpp
    gui/searchconfig/intelligentretrieval/detailcheckbox.cpp
    gui/searchconfig/intelligentretrieval/detailcheckbox.h
    gui/searchconfig/intelligentretrieval/autoindexstatus.cpp
    gui/searchconfig/intelligentretrieval/autoindexstatus.h
    gui/searchconfig/tipslabel.h
    gui/searchconfig/tipslabel.cpp
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
    # 配置读写
    business/config/searchconfig.cpp
    business/config/searchconfig.h
    # 访问记录
    business/config/accessrecord/accessrecord.h
    business/config/accessrecord/accessrecord.cpp
    )

# contacts
set(CONTACTSSRC
    # dbus服务
    contacts/services/grandsearchservice_p.h
    contacts/services/grandsearchservice.h
    contacts/services/grandsearchservice.cpp
    )

# utils
set(UTILSSRC
    utils/utils.h
    utils/utils.cpp
    utils/previewpluginconf.h
    utils/filestatisticsthread.h
    utils/filestatisticsthread.cpp
    utils/report/basicpoint.h
    utils/report/basicpoint.cpp
    utils/report/committhread.h
    utils/report/committhread.cpp
    utils/report/eventlogutil.h
    utils/report/eventlogutil.cpp
    utils/report/aisearchpoint.h
    )

# 源文件
set(SRCS
    main.cpp
    ${GLOBAL_SRC}
    ${GUISRC}
    ${BUSINESSSRC}
    ${CONTACTSSRC}
    ${UTILSSRC}
    ${DBUSSRC}
)

# 指定资源文件
set(QRCS
    gui/searchconfig/resource.qrc
    gui/exhibition/preview/resource.qrc
    gui/exhibition/matchresult/resource.qrc
    )

add_executable(${BIN_NAME} ${SRCS} ${QRCS})

target_include_directories(${BIN_NAME} PUBLIC
    ${CMAKE_SOURCE_DIR}/include
    ${GIO_LIB_INCLUDE_DIRS}
)

target_link_libraries(${BIN_NAME} PRIVATE
    ${Qt_LIBS}
    ${DTK_LIBS}
    ${GIO_LIB_LIBRARIES}
)

set(SRCS_SETTING
    setting_main.cpp
    contacts/services/grandsearchsettingservice_p.h
    contacts/services/grandsearchsettingservice.h
    contacts/services/grandsearchsettingservice.cpp
    ${PROJECT_3RDPARTY_PATH}/services/grandsearchsettingserviceadaptor.h
    ${PROJECT_3RDPARTY_PATH}/services/grandsearchsettingserviceadaptor.cpp
)

add_executable(${BIN_NAME}-setting ${SRCS_SETTING})
target_link_libraries(${BIN_NAME}-setting PRIVATE
    ${Qt_LIBS}
    ${DTK_LIBS}
)

# Install settings
if (CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(CMAKE_INSTALL_PREFIX /usr)
endif ()

# bin
install(TARGETS ${BIN_NAME} DESTINATION bin)
install(TARGETS ${BIN_NAME}-setting DESTINATION bin)
# ts
install(DIRECTORY ${PROJECT_SOURCE_DIR}/translations
    DESTINATION share/${BIN_NAME}
    FILES_MATCHING PATTERN "*.qm")

# dbus
install(FILES contacts/services/com.deepin.dde.GrandSearch.service DESTINATION ${CMAKE_INSTALL_DATADIR}/dbus-1/services)
install(FILES contacts/services/com.deepin.dde.GrandSearch.xml DESTINATION ${CMAKE_INSTALL_DATADIR}/dbus-1/interfaces)

# setting
install(FILES contacts/services/com.deepin.dde.GrandSearchSetting.service DESTINATION ${CMAKE_INSTALL_DATADIR}/dbus-1/services)
install(FILES contacts/services/com.deepin.dde.GrandSearchSetting.xml DESTINATION ${CMAKE_INSTALL_DATADIR}/dbus-1/interfaces)

# manual
install(DIRECTORY ${CMAKE_SOURCE_DIR}/assets/manual/dde-grand-search
    DESTINATION share/deepin-manual/manual-assets/application)

# desktop
install(FILES dde-grand-search.desktop DESTINATION share/applications)
