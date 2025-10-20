// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "environments.h"
#include "global/grandsearch_global.h"
#include "global/framelogmanager.h"

#include "gui/mainwindow.h"
#include "gui/searchconfig/configwidget.h"
#include "business/query/querycontroller.h"
#include "business/matchresult/matchcontroller.h"
#include "contacts/services/grandsearchservice.h"
#include "business/config/accessrecord/accessrecord.h"

#include "services/grandsearchserviceadaptor.h"

#include <DApplication>
#include <DWidgetUtil>
#include <DLog>

#include <QDebug>
#include <QTimer>
#include <QtDBus/QDBusConnection>
#include <QtDBus/QDBusError>

#include <unistd.h>

Q_LOGGING_CATEGORY(logGrandSearch, "org.deepin.dde.GrandSearch.GrandSearch")

GRANDSEARCH_USE_NAMESPACE
DGUI_USE_NAMESPACE
DWIDGET_USE_NAMESPACE
DCORE_USE_NAMESPACE

int main(int argc, char *argv[])
{
    DApplication app(argc, argv);

    // 设置应用信息
    app.setOrganizationName("deepin");
    app.setApplicationName("dde-grand-search");
    app.setApplicationDisplayName("DDE Grand Search");
    app.setApplicationVersion(APP_VERSION);
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    app.setAttribute(Qt::AA_UseHighDpiPixmaps);
#endif

    // 设置终端和文件记录日志
    dgsLogManager->applySuggestedLogSettings();

    QCommandLineParser parser;

    bool isSetting = false;
    QString position;

    if (argc > 1) {
        QCommandLineOption option_setting({ "s", "setting" }, "Start grand search config.");
        QCommandLineOption option_position("position", "Start grand search config, and scroll to one position.", "value");

        parser.addOption(option_setting);
        parser.addOption(option_position);
        parser.addHelpOption();

        parser.process(app);
        isSetting = parser.isSet(option_setting);
        position = parser.value(option_position);
    }
    // 根据要求，通过启动参数决定显示界面
    if (isSetting) {   // 设置界面
        // 设置单例运行程序
        if (!app.setSingleInstance("dde-grand-search-config")) {
            qCWarning(logGrandSearch) << "Failed to set single instance mode - PID:" << getpid()
                                      << "Application will exit";
            return -1;
        }

        // 加载翻译
        app.loadTranslator();

        qCInfo(logGrandSearch) << "Starting configuration interface - Application:" << app.applicationName()
                               << "Version:" << app.applicationVersion()
                               << "PID:" << getpid();

        ConfigWidget w;
        w.show();
        if (position == "aiconfig") {
            qCDebug(logGrandSearch) << "Scrolling to AI configuration section";
            w.scrollToAiConfig();
        }
        Dtk::Widget::moveToCenter(&w);

        // 重复启动时，激活已有窗口
        QObject::connect(&app, &DApplication::newInstanceStarted, &w, &ConfigWidget::activateWindow);

        return app.exec();
    } else {   // 搜索界面
        // 设置单例运行程序
        if (!app.setSingleInstance("dde-grand-search")) {
            qCWarning(logGrandSearch) << "Failed to set single instance mode - PID:" << getpid()
                                      << "Application will exit";
            return -1;
        }

        // 加载翻译
        app.loadTranslator();

        qCInfo(logGrandSearch) << "Starting search interface - Application:" << app.applicationName()
                               << "Version:" << app.applicationVersion()
                               << "PID:" << getpid();

        // 解析点选数据
        AccessRecord::instance()->startParseRecord();
        QObject::connect(&app, &QCoreApplication::aboutToQuit, AccessRecord::instance(), &AccessRecord::sync);

        MainWindow mainWindow;
        mainWindow.show();

        QTimer::singleShot(0, &mainWindow, [&mainWindow]() {
            // 界面初始化完成后，再处理与业务有关的连接
            mainWindow.connectToController();
        });

        // 注册dbus服务
        QDBusConnection conn = QDBusConnection::sessionBus();
        if (!conn.isConnected()) {
            qCCritical(logGrandSearch) << "Failed to connect to D-Bus session bus";
            return -1;
        }

        GrandSearchService service(&mainWindow);
        Q_UNUSED(new GrandSearchServiceAdaptor(&service))
        if (!conn.registerService(GrandSearchViewServiceName)) {
            qCCritical(logGrandSearch) << "Failed to register D-Bus service:" << conn.lastError().message();
            return -1;
        }

        if (!conn.registerObject(GrandSearchViewServicePath, &service)) {
            qCCritical(logGrandSearch) << "Failed to register D-Bus object:" << conn.lastError().message();
            return -1;
        }

        qCInfo(logGrandSearch) << "D-Bus service registered successfully - Service:" << GrandSearchViewServiceName
                               << "Path:" << GrandSearchViewServicePath;

        return app.exec();
    }
}
