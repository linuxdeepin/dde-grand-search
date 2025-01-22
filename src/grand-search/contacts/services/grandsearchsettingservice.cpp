// SPDX-FileCopyrightText: 2021 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "grandsearchsettingservice_p.h"
#include "grandsearchsettingservice.h"

#include <QProcess>
#include <QApplication>
using namespace GrandSearchSetting;

GrandSearchSettingServicePrivate::GrandSearchSettingServicePrivate(GrandSearchSettingService *parent)
    : q_p(parent)
{
    timer = new QTimer(parent);
    timer->setInterval(5000);
    timer->setSingleShot(true);
    connect(timer, &QTimer::timeout, this, [this]() {
        qApp->exit(0);
        _Exit(0);
    });
    timer->start();
}

GrandSearchSettingService::GrandSearchSettingService(QObject *parent)
    : QObject(parent)
    , d_p(new GrandSearchSettingServicePrivate(this))
{
}

GrandSearchSettingService::~GrandSearchSettingService()
{

}

void GrandSearchSettingService::Show()
{
    if (!qEnvironmentVariableIsEmpty("XDG_SESSION_TYPE") 
        && qEnvironmentVariable("XDG_SESSION_TYPE") == QStringLiteral("x11")) {
        qputenv("QT_QPA_PLATFORM", "dxcb;xcb");
    }

    d_p->timer->stop();
    QProcess process;
    process.start("dde-grand-search", QStringList()<<"--setting");
    process.waitForFinished(-1);
    qApp->exit(0);
    _Exit(0);
}
