// SPDX-FileCopyrightText: 2021 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "handlevisibility.h"

#include "gui/mainwindow.h"

#include <DApplication>

#include <QtDebug>
#include <QTimer>
#include <QLoggingCategory>

Q_DECLARE_LOGGING_CATEGORY(logGrandSearch)

#define SessionManagerService "org.deepin.dde.SessionManager1"
#define SessionManagerPath "/org/deepin/dde/SessionManager1"

using namespace GrandSearch;
DWIDGET_USE_NAMESPACE

HandleVisibility::HandleVisibility(MainWindow *mainWindow, QObject *parent)
    : QObject(parent),
      m_mainWindow(mainWindow)
// , m_sessionManagerInter(new com::deepin::SessionManager(SessionManagerService, SessionManagerPath, QDBusConnection::sessionBus(), this))
{
    qCDebug(logGrandSearch) << "Creating HandleVisibility";
    init();
    qCDebug(logGrandSearch) << "HandleVisibility created successfully";
}

void HandleVisibility::init()
{
    qCDebug(logGrandSearch) << "Initializing visibility handler";

    m_regionMonitor = new DRegionMonitor(this);
    m_regionMonitor->setCoordinateType(DRegionMonitor::ScaleRatio);
    // 鼠标点击主界面之外的区域，退出进程
    connect(m_regionMonitor, &DRegionMonitor::buttonPress, this, &HandleVisibility::regionMousePress);
    registerRegion(true);

    // 进程变为非激活状态时，退出
    connect(qApp, &QGuiApplication::applicationStateChanged, this, &HandleVisibility::onApplicationStateChanged);

    // 锁屏状态改变
    // FIXME: how to replace this feature on V25
    // connect(m_sessionManagerInter, &com::deepin::SessionManager::LockedChanged, this, &HandleVisibility::onLockedChanged);
}

void HandleVisibility::onApplicationStateChanged(const Qt::ApplicationState state)
{
    Q_ASSERT(m_mainWindow);

    if (Qt::ApplicationInactive == state) {
        // 呼出宽限期：主搜索框刚显示（如保存/打开文件对话框等模态场景下
        // XSetInputFocus 抢焦点存在窗口期）时，500ms 内不因失焦关闭，
        // 避免主搜索框一显示就被判定为失焦而立即消失
        if (m_mainWindow->isVisible() && m_mainWindow->isGracePeriodActive()) {
            qCDebug(logGrandSearch) << "Application inactive within grace period - keep main window";
            return;
        }
        // 模态窗口场景：呼出后主窗从未获得过激活（焦点被保存/打开文件对话框
        // 等模态窗口锁死，XSetInputFocus 抢焦点失败的兜底）——保持主窗显示，
        // 直到用户主动关闭或插入路径，而不是一失焦就消失
        if (m_mainWindow->isVisible() && m_mainWindow->neverActivatedSinceShow()) {
            qCDebug(logGrandSearch) << "Application inactive with modal dialog - keep main window visible";
            return;
        }
        // 主搜索框失焦 = 随用随关：直接关闭（与官方行为一致）
        qCInfo(logGrandSearch) << "Application state changed to inactive - Closing main window";
        m_mainWindow->close();
    } else if (Qt::ApplicationActive == state) {
        m_mainWindow->markActivated();   // 记录已获得激活（区分模态锁焦与用户点击失焦）
        qCDebug(logGrandSearch) << "Application state changed to active - Showing main window";
        m_mainWindow->show();
    }
}

void HandleVisibility::onLockedChanged(const bool locked)
{
    Q_ASSERT(m_mainWindow);

    qCDebug(logGrandSearch) << "Screen lock state changed - Locked:" << locked
                            << "Setting window visible:" << !locked;
    m_mainWindow->setVisible(!locked);
}

void HandleVisibility::onCloseWindow()
{
    Q_ASSERT(m_mainWindow);

    qCDebug(logGrandSearch) << "Closing main window";
    // 主搜索框关闭（Esc / 点击结果条目）= 随用随关：直接关闭（与官方行为一致）。
    // 关闭后的去向由 MainWindow::closeEvent 决定（面板有会话仅隐藏，无会话退出）
    m_mainWindow->close();
}

void HandleVisibility::registerRegion(const bool isRegister)
{
    Q_ASSERT(m_regionMonitor);

    if (isRegister == m_regionMonitor->registered())
        return;

    qCDebug(logGrandSearch) << "Region registration:" << isRegister;
    // FIXME: how to fix blocked 25s issue on treeland
    /*
    if (isRegister)
        m_regionMonitor->registerRegion();
    else
        m_regionMonitor->unregisterRegion();
    */
}

void HandleVisibility::regionMousePress(const QPoint &p, const int flag)
{
    Q_UNUSED(flag)

    for (auto w : qApp->topLevelWidgets()) {
        if (w->isVisible() && w->geometry().contains(p)) {
            return;
        }
    }
    // 点击位置在程序窗口之外则关闭主搜索框（与官方行为一致，随用随关）
    qCInfo(logGrandSearch) << "Mouse click outside application window - Position:" << p
                           << "Flag:" << flag
                           << "Scheduling window close";
    // 延迟隐藏，便于dock插件响应时能正确获取当前状态
    QTimer::singleShot(500, this, &HandleVisibility::onCloseWindow);
}
