// SPDX-FileCopyrightText: 2026 testerxydw
// SPDX-License-Identifier: GPL-3.0-or-later

#include "convertsearch.h"
#include "searchpluginadaptor.h"

#include <QGuiApplication>
#include <QDBusConnection>
#include <QDBusError>
#include <QLoggingCategory>

#define DBUS_SERVICE_NAME "org.deepin.dde.GrandSearchConvert"
#define DBUS_OBJECT_PATH  "/org/deepin/dde/GrandSearchConvert"

int main(int argc, char *argv[])
{
    // 守护进程运行在用户会话中，使用 QGuiApplication 以便通过 Qt clipboard
    // 或 deepin 剪贴板 DBus 服务写入系统剪贴板（点击复制 / UOS AI 传内容）。
    // 无显示环境（CI/headless）默认 offscreen，保证进程仍可启动（剪贴板走 DBus 回退）。
    if (qEnvironmentVariableIsEmpty("QT_QPA_PLATFORM")
        && qEnvironmentVariableIsEmpty("DISPLAY")
        && qEnvironmentVariableIsEmpty("WAYLAND_DISPLAY")) {
        qputenv("QT_QPA_PLATFORM", "offscreen");
    }
    QGuiApplication app(argc, argv);
    app.setApplicationName("convert-search-plugin");
    QLoggingCategory::setFilterRules("convert.search.plugin.debug=true");

    ConvertSearch searcher;
    SearchPluginAdaptor adaptor(&searcher);

    QDBusConnection connection = QDBusConnection::sessionBus();
    if (!connection.registerService(DBUS_SERVICE_NAME)) {
        qCritical() << "Failed to register DBus service:" << DBUS_SERVICE_NAME
                    << connection.lastError().message();
        return 1;
    }
    if (!connection.registerObject(DBUS_OBJECT_PATH, &searcher)) {
        qCritical() << "Failed to register DBus object:" << DBUS_OBJECT_PATH;
        return 1;
    }

    qInfo() << "Convert search plugin started - Service:" << DBUS_SERVICE_NAME
            << "Path:" << DBUS_OBJECT_PATH;
    return app.exec();
}
