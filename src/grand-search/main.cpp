/*
 * Copyright (C) 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhangyu<zhangyub@uniontech.com>
 *
 * Maintainer: wangchunlin<wangchunlin@uniontech.com>
 *             zhangyu<zhangyub@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "environments.h"

#include "gui/mainwindow.h"
#include "gui/searchconfig/configwidget.h"
#include "business/query/querycontroller.h"
#include "business/matchresult/matchcontroller.h"
#include "contacts/services/grandsearchservice.h"
#include "contacts/services/grandsearchserviceadaptor.h"
#include "business/config/accessrecord/accessrecord.h"

#include <DApplication>
#include <DWidgetUtil>
#include <DLog>

#include <QDebug>
#include <QTimer>
#include <QtDBus/QDBusConnection>
#include <QtDBus/QDBusError>

#include <unistd.h>

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
    app.setApplicationVersion(VERSION);
    app.setAttribute(Qt::AA_UseHighDpiPixmaps);

    // 设置终端和文件记录日志
    const QString logFormat = "%{time}{yyyyMMdd.HH:mm:ss.zzz}[%{type:1}][%{function:-35} %{line:-4} %{threadid} ] %{message}\n";
    DLogManager::setLogFormat(logFormat);
    DLogManager::registerConsoleAppender();
    DLogManager::registerFileAppender();

    QCommandLineParser parser;

    bool isSetting = false;

    if (argc > 1) {
        QCommandLineOption option_setting({"s", "setting"}, "Start grand search config.");

        parser.addOption(option_setting);
        parser.addHelpOption();

        parser.process(app);
        isSetting = parser.isSet(option_setting);
    }
    // 根据要求，通过启动参数决定显示界面
    if (isSetting) { // 设置界面
        // 设置单例运行程序
        if (!app.setSingleInstance("dde-grand-search-config")) {
            qWarning() << "set single instance failed!I (pid:" << getpid() << ") will exit.";
            return -1;
        }

        // 加载翻译
        app.loadTranslator();

        qDebug() << "starting config:" << app.applicationName() << app.applicationVersion() << getpid();

        ConfigWidget w;
        w.show();
        Dtk::Widget::moveToCenter(&w);

        // 重复启动时，激活已有窗口
        QObject::connect(&app, &DApplication::newInstanceStarted, &w, &ConfigWidget::activateWindow);

        return app.exec();
    } else {    // 搜索界面
        // 设置单例运行程序
        if (!app.setSingleInstance("dde-grand-search")) {
            qWarning() << "set single instance failed!I (pid:" << getpid() << ") will exit.";
            return -1;
        }

        // 加载翻译
        app.loadTranslator();

        qDebug() << "starting search:" << app.applicationName() << app.applicationVersion() << getpid();

        // 解析点选数据
        AccessRecord::instance()->startParseRecord();
        QObject::connect(&app, &QCoreApplication::aboutToQuit, AccessRecord::instance(), &AccessRecord::sync);

        MainWindow mainWindow;
        mainWindow.show();

        QTimer::singleShot(0, &mainWindow, [&mainWindow](){
            // 界面初始化完成后，再处理与业务有关的连接
            mainWindow.connectToController();
        });

        // 注册dbus服务
        QDBusConnection conn = QDBusConnection::sessionBus();
        if (!conn.isConnected()) {
            qWarning() << "QDBusConnection is not connect.";
            return -1;
        }

        GrandSearchService service(&mainWindow);
        Q_UNUSED(new GrandSearchServiceAdaptor(&service))
        if (!conn.registerService(GrandSearchViewServiceName)) {
            qWarning() << "registerService Failed:" << conn.lastError();
            return -1;
        }

        if (!conn.registerObject(GrandSearchViewServicePath, &service)) {
            qWarning() << "registerObject Failed:" << conn.lastError();
            return -1;
        }

        return app.exec();
    }
}
