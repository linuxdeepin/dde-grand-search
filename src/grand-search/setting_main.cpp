// SPDX-FileCopyrightText: 2021 - 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "environments.h"
#include "contacts/services/grandsearchsettingservice.h"

#include "services/grandsearchsettingserviceadaptor.h"

#include <DApplication>

#include <QDBusConnection>
#include <QLoggingCategory>

Q_LOGGING_CATEGORY(logGrandSearch, "org.deepin.dde.GrandSearch.GrandSearch")

DWIDGET_USE_NAMESPACE
using namespace GrandSearchSetting;

int main(int argc, char *argv[])
{
    DApplication app(argc, argv);

    // 设置应用信息
    app.setOrganizationName("deepin");
    app.setApplicationName("dde-grand-search-setting");
    app.setApplicationDisplayName("DDE Grand Search Setting");
    app.setApplicationVersion(APP_VERSION);

    qCInfo(logGrandSearch) << "Starting DDE Grand Search Setting service - Version:" << APP_VERSION;

    if (!app.setSingleInstance("dde-grand-search-setting")) {
        qCWarning(logGrandSearch) << "Failed to set single instance - Another setting service may be running";
        return -1;
    }
    qCDebug(logGrandSearch) << "Single instance mode set successfully";

    // 注册dbus服务
    QDBusConnection conn = QDBusConnection::sessionBus();
    if (!conn.isConnected()) {
        qCCritical(logGrandSearch) << "Failed to connect to D-Bus session bus";
        return -1;
    }
    qCDebug(logGrandSearch) << "Connected to D-Bus session bus successfully";

    GrandSearchSettingService service;
    Q_UNUSED(new GrandSearchSettingServiceAdaptor(&service))

    if (!conn.registerService(GrandSearchSettingServiceName)) {
        qCCritical(logGrandSearch) << "Failed to register D-Bus service:" << GrandSearchSettingServiceName
                                   << "Error:" << conn.lastError().message();
        return -1;
    }
    qCDebug(logGrandSearch) << "D-Bus service registered successfully:" << GrandSearchSettingServiceName;

    if (!conn.registerObject(GrandSearchSettingServicePath, &service)) {
        qCCritical(logGrandSearch) << "Failed to register D-Bus object:" << GrandSearchSettingServicePath
                                   << "Error:" << conn.lastError().message();
        return -1;
    }
    qCDebug(logGrandSearch) << "D-Bus object registered successfully:" << GrandSearchSettingServicePath;

    qCInfo(logGrandSearch) << "DDE Grand Search Setting service started and ready";

    return app.exec();
}
