// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "environments.h"
#include "dbusservice/grandsearchinterface.h"
#include "global/grandsearch_global.h"
#include "global/framelogmanager.h"

#include <DLog>
#include <DApplication>

#include <QCoreApplication>
#include <QDBusConnection>
#include <QDBusError>
#include <QDebug>
#include <QThread>
#include <QDir>

#include <unistd.h>
#include <signal.h>

Q_LOGGING_CATEGORY(logDaemon, "org.deepin.dde.GrandSearch.Daemon")

GRANDSEARCH_USE_NAMESPACE
DCORE_USE_NAMESPACE
DWIDGET_USE_NAMESPACE
DGUI_USE_NAMESPACE

static GrandSearchInterface *instance = nullptr;

#define DGSLIB_VERSION_MAJOR 1
#define DGSLIB_VERSION_MINOR 0
#define DGSLIB_VERSION_PATCH 0

#ifdef __cplusplus
extern "C" {
#endif

int startGrandSearchDaemon(int argc, char *argv[])
{
    if (instance) {
        qCWarning(logDaemon) << "Service already running: grand search daemon instance exists";
        return 0;
    }

    if (qApp == nullptr) {
        qCCritical(logDaemon) << "Failed to initialize: Qt Application not initialized";
        return 1;
    }

    if (QThread::currentThread() != qApp->thread()) {
        qCCritical(logDaemon) << "Invalid thread: startGrandSearchDaemon must be called in main thread";
        return 2;
    }

    // 服务接口
    GrandSearchInterface *interface = new GrandSearchInterface();

    // 注册DBus服务
    {
        QDBusConnection session = interface->qDbusConnection();
        if (!session.registerService(GrandSearchServiceName)) {
            qCCritical(logDaemon) << "DBus service registration failed:" << GrandSearchServiceName
                                     << "Error:" << QDBusError::errorString(session.lastError().type());
            delete interface;
            return 3;
        }

        if (!session.registerObject(GrandSearchServicePath, interface,
                                    QDBusConnection::ExportScriptableSlots
                                            | QDBusConnection::ExportScriptableSignals
                                            | QDBusConnection::ExportScriptableProperties)) {
            qCCritical(logDaemon) << "DBus object registration failed:" << GrandSearchServicePath
                                     << "Error:" << QDBusError::errorString(session.lastError().type());
            delete interface;
            return 4;
        }
    }

    // 初始化
    if (!interface->init()) {
        qCCritical(logDaemon) << "Failed to initialize grand search daemon. Version:" << APP_VERSION;
        return -1;
    }

    instance = interface;
    qCInfo(logDaemon) << "Grand search daemon started successfully. Version:" << APP_VERSION;

    // 加载翻译
    QString appName = qApp->applicationName();
    qApp->setApplicationName("dde-grand-search");
    qApp->loadTranslator();
    qApp->setApplicationName(appName);

    return 0;
}

int stopGrandSearchDaemon()
{
    if (instance) {
        qCInfo(logDaemon) << "Stopping grand search daemon";
        delete instance;
        instance = nullptr;
    }
    return 0;
}

const char *grandSearchDaemonAppVersion()
{
    return APP_VERSION;
}

int grandSearchDaemonLibVersion()
{
    return QT_VERSION_CHECK(DGSLIB_VERSION_MAJOR, DGSLIB_VERSION_MINOR, DGSLIB_VERSION_PATCH);
}

static void appExitHandler(int sig)
{
    qCInfo(logDaemon) << "Received exit signal:" << sig;

    // 释放资源，退出插件子进程。
    qApp->quit();
}

int main(int argc, char *argv[])
{
    // 安全退出
    signal(SIGINT, appExitHandler);
    signal(SIGQUIT, appExitHandler);
    signal(SIGTERM, appExitHandler);

    DApplication app(argc, argv);

    {
        QString dateTime = QDateTime::currentDateTime().toString(Qt::ISODateWithMs);
        // 设置应用信息
        app.setOrganizationName("deepin");
        app.setApplicationName("dde-grand-search-daemon");
        app.setApplicationVersion(APP_VERSION);

        // 日志
        dgsLogManager->applySuggestedLogSettings();
        qCInfo(logDaemon) << dateTime << "Starting" << app.applicationName()
                             << "Version:" << app.applicationVersion()
                             << "PID:" << getpid();
    }

    // 初始化
    int ret = startGrandSearchDaemon(argc, argv);
    if (ret != 0) {
        qCCritical(logDaemon) << "Daemon initialization failed with error code:" << ret;
        return ret;
    }
    ret = app.exec();
    stopGrandSearchDaemon();

    return ret;
}

#ifdef __cplusplus
}
#endif
