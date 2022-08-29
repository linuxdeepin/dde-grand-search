// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "environments.h"
#include "dbusservice/grandsearchinterface.h"

#include <DLog>
#include <DApplication>

#include <QCoreApplication>
#include <QDBusConnection>
#include <QDBusError>
#include <QDebug>

#include <unistd.h>
#include <signal.h>

DCORE_USE_NAMESPACE
DWIDGET_USE_NAMESPACE

static void appExitHandler(int sig) {
    qInfo() << "signal" << sig << "exit.";

    //释放资源，退出插件子进程。
    qApp->quit();
}

int main(int argc, char *argv[])
{
    //安全退出
    signal(SIGINT, appExitHandler);
    signal(SIGQUIT, appExitHandler);
    signal(SIGTERM, appExitHandler);

    DApplication app(argc, argv);

    {
        QString dateTime = QDateTime::currentDateTime().toString(Qt::ISODateWithMs);
        //设置应用信息
        app.setOrganizationName("deepin");
        app.setApplicationName("dde-grand-search-daemon");
        app.setApplicationVersion(VERSION);

        // 设置终端和文件记录日志
        const QString logFormat = "%{time}{yyyyMMdd.HH:mm:ss.zzz}[%{type:1}][%{function:-35} %{line:-4} %{threadid} ] %{message}\n";
        DLogManager::setLogFormat(logFormat);
        DLogManager::registerConsoleAppender();
        DLogManager::registerFileAppender();
        qInfo() << dateTime << "starting " << app.applicationName() << app.applicationVersion() << getpid();
    }

    //服务接口
    GrandSearchInterface interface;

    //注册DBus服务
    {
        QDBusConnection session = QDBusConnection::sessionBus();
        if (!session.registerService(GrandSearchServiceName)) {
            qCritical() << "registerService Failed, maybe service exist" << QDBusError::errorString(session.lastError().type());
            exit(0x0002);
        }

        if (!session.registerObject(GrandSearchServicePath, &interface,
                                 QDBusConnection::ExportScriptableSlots |
                                 QDBusConnection::ExportScriptableSignals |
                                 QDBusConnection::ExportScriptableProperties)) {
            qCritical() << "registerObject Failed" << QDBusError::errorString(session.lastError().type());
            exit(0x0003);
        }
    }

    //初始化
    if (!interface.init()) {
        qCritical() << "initialization failed.";
        exit(0x0001);
    }

    {
        // 加载翻译
        QString appName = app.applicationName();
        app.setApplicationName("dde-grand-search");
        app.loadTranslator();
        app.setApplicationName(appName);
    }

    return app.exec();
}
