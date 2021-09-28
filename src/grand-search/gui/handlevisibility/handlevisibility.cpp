/*
 * Copyright (C) 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     wangchunlin<wangchunlin@uniontech.com>
 *
 * Maintainer: wangchunlin<wangchunlin@uniontech.com>
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
#include "handlevisibility.h"
#include "gui/mainwindow.h"

#include <DApplication>

#include <QtDebug>

#define SessionManagerService "com.deepin.SessionManager"
#define SessionManagerPath "/com/deepin/SessionManager"

DWIDGET_USE_NAMESPACE
using namespace com::deepin;

HandleVisibility::HandleVisibility(MainWindow *mainWindow, QObject *parent)
    : QObject(parent)
    , m_mainWindow(mainWindow)
    , m_sessionManagerInter(new SessionManager(SessionManagerService, SessionManagerPath, QDBusConnection::sessionBus(), this))
{
    init();
}

void HandleVisibility::init()
{
    m_regionMonitor = new DRegionMonitor(this);
    m_regionMonitor->setCoordinateType(DRegionMonitor::ScaleRatio);
    // 鼠标点击主界面之外的区域，退出进程
    connect(m_regionMonitor, &DRegionMonitor::buttonPress, this, &HandleVisibility::regionMousePress);
    registerRegion(true);

    // 进程变为非激活状态时，退出
    connect(qApp, &QGuiApplication::applicationStateChanged, this, &HandleVisibility::onApplicationStateChanged);

    // 锁屏状态改变
    connect(m_sessionManagerInter, &SessionManager::LockedChanged, this, &HandleVisibility::onLockedChanged);
}

void HandleVisibility::onApplicationStateChanged(const Qt::ApplicationState state)
{
    Q_ASSERT(m_mainWindow);

    if (Qt::ApplicationInactive == state) {
        // todo 二阶段：截图导致本进程窗口变为非激活，不退出。截图退出后，主动激活本进程主窗口。
        qDebug() << "application state change to inactive,so i will exit.";
        m_mainWindow->close();
    } else if (Qt::ApplicationActive == state) {
        qDebug() << "application state change to active.";
        m_mainWindow->show();
    }
}

void HandleVisibility::onLockedChanged(const bool locked)
{
    Q_ASSERT(m_mainWindow);

    m_mainWindow->setVisible(!locked);
}

void HandleVisibility::onCloseWindow()
{
    Q_ASSERT(m_mainWindow);

    m_mainWindow->close();
}

void HandleVisibility::registerRegion(const bool isRegister)
{
    Q_ASSERT(m_regionMonitor);

    if (isRegister == m_regionMonitor->registered())
        return;
    if (isRegister)
        m_regionMonitor->registerRegion();
    else
        m_regionMonitor->unregisterRegion();
}

void HandleVisibility::regionMousePress(const QPoint &p, const int flag)
{
    Q_UNUSED(flag)

    for (auto w : qApp->topLevelWidgets()) {
        if (w->isVisible() && w->geometry().contains(p)) {
            return;
        }
    }
    // 点击位置在程序窗口之外则退出
    qDebug() << "click:" << p << "   flag:" << flag << "    and exit.";
    // todo ：后期根据情况，可能需要设置为隐藏，而不能直接退出
    // 延迟退出，便于dock插件响应时能正确获取当前状态
    QTimer::singleShot(500, this, &HandleVisibility::onCloseWindow);
}
