// SPDX-FileCopyrightText: 2021 - 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "environments.h"
#include "contacts/services/grandsearchsettingservice.h"

#include "services/grandsearchsettingserviceadaptor.h"

#include <DApplication>

#include <QDBusConnection>

DWIDGET_USE_NAMESPACE
using namespace GrandSearchSetting;

int main(int argc, char *argv[])
{
    DApplication app(argc, argv);

    // 设置应用信息
    app.setOrganizationName("deepin");
    app.setApplicationName("dde-grand-search-setting");
    app.setApplicationDisplayName("DDE Grand Search Setting");
    app.setApplicationVersion(VERSION);

    if (!app.setSingleInstance("dde-grand-search-setting")) {
        return -1;
    }

    // 注册dbus服务
    QDBusConnection conn = QDBusConnection::sessionBus();
    if (!conn.isConnected()) {
        qWarning() << "QDBusConnection is not connect.";
        return -1;
    }

    GrandSearchSettingService service;
    Q_UNUSED(new GrandSearchSettingServiceAdaptor(&service))
    if (!conn.registerService(GrandSearchSettingServiceName)) {
        qWarning() << "registerService Failed:" << conn.lastError();
        return -1;
    }

    if (!conn.registerObject(GrandSearchSettingServicePath, &service)) {
        qWarning() << "registerObject Failed:" << conn.lastError();
        return -1;
    }

    return app.exec();
}
