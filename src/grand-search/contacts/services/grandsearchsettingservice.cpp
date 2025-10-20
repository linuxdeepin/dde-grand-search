// SPDX-FileCopyrightText: 2021 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "grandsearchsettingservice_p.h"
#include "grandsearchsettingservice.h"

#include <QProcess>
#include <QApplication>
#include <QLoggingCategory>

Q_DECLARE_LOGGING_CATEGORY(logGrandSearch)

using namespace GrandSearchSetting;

GrandSearchSettingServicePrivate::GrandSearchSettingServicePrivate(GrandSearchSettingService *parent)
    : q_p(parent)
{
    qCDebug(logGrandSearch) << "GrandSearchSettingServicePrivate created - Setting up auto-exit timer";

    timer = new QTimer(parent);
    timer->setInterval(5000);
    timer->setSingleShot(true);
    connect(timer, &QTimer::timeout, this, [this]() {
        qCInfo(logGrandSearch) << "Setting service auto-exit timer triggered - Shutting down after 5 seconds";
        qApp->exit(0);
        _Exit(0);
    });
    timer->start();
    qCDebug(logGrandSearch) << "Auto-exit timer started - 5 seconds interval";
}

GrandSearchSettingService::GrandSearchSettingService(QObject *parent)
    : QObject(parent)
    , d_p(new GrandSearchSettingServicePrivate(this))
{
    qCDebug(logGrandSearch) << "GrandSearchSettingService initialized";
}

GrandSearchSettingService::~GrandSearchSettingService()
{
    qCDebug(logGrandSearch) << "GrandSearchSettingService destroyed";
}

void GrandSearchSettingService::Show()
{
    qCInfo(logGrandSearch) << "D-Bus method called: Show - Launching settings interface";

    // 检查会话类型并设置适当的平台插件
    if (!qEnvironmentVariableIsEmpty("XDG_SESSION_TYPE")
        && qEnvironmentVariable("XDG_SESSION_TYPE") == QStringLiteral("x11")) {
        qCDebug(logGrandSearch) << "X11 session detected - Setting QT_QPA_PLATFORM to dxcb;xcb";
        qputenv("QT_QPA_PLATFORM", "dxcb;xcb");
    }

    d_p->timer->stop();
    QProcess process;
    qCDebug(logGrandSearch) << "Starting settings process: dde-grand-search --setting";
    process.start("dde-grand-search", QStringList()<<"--setting");
    process.waitForFinished(-1);
    qApp->exit(0);
    _Exit(0);
}
