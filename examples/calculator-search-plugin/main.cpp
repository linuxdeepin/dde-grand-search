// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "calculatorsearch.h"
#include "searchpluginadaptor.h"

#include <QCoreApplication>
#include <QDBusConnection>
#include <QLoggingCategory>

#define DBUS_SERVICE_NAME   "com.example.CalculatorSearch"
#define DBUS_OBJECT_PATH    "/com/example/CalculatorSearch"

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    app.setApplicationName("calculator-search-plugin");

    QLoggingCategory::setFilterRules("calculator.search.plugin.debug=true");

    CalculatorSearch searcher;

    SearchPluginAdaptor adaptor(&searcher);

    QDBusConnection connection = QDBusConnection::sessionBus();
    if (!connection.registerService(DBUS_SERVICE_NAME)) {
        qCritical("Failed to register DBus service: %s", DBUS_SERVICE_NAME);
        return 1;
    }

    if (!connection.registerObject(DBUS_OBJECT_PATH, &searcher)) {
        qCritical("Failed to register DBus object: %s", DBUS_OBJECT_PATH);
        return 1;
    }

    qInfo("Calculator search plugin started - Service: %s, Path: %s",
          DBUS_SERVICE_NAME, DBUS_OBJECT_PATH);

    return app.exec();
}
