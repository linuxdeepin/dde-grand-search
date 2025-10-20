// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "grandsearchservice_p.h"
#include "grandsearchservice.h"
#include "gui/mainwindow.h"

#include <QDebug>
#include <QLoggingCategory>

Q_DECLARE_LOGGING_CATEGORY(logGrandSearch)

using namespace GrandSearch;

GrandSearchServicePrivate::GrandSearchServicePrivate(MainWindow *mainWindow, GrandSearchService *parent)
    : q_p(parent),
      m_mainWindow(mainWindow)
{
    qCDebug(logGrandSearch) << "GrandSearchServicePrivate created with MainWindow:" << (mainWindow != nullptr);
}

GrandSearchService::GrandSearchService(MainWindow *mainWindow, QObject *parent)
    : QObject(parent), d_p(new GrandSearchServicePrivate(mainWindow, this))
{
    qCDebug(logGrandSearch) << "Initializing GrandSearchService - MainWindow:" << (mainWindow != nullptr);
    connect(d_p->m_mainWindow, &MainWindow::visibleChanged, this, &GrandSearchService::VisibleChanged);
}

GrandSearchService::~GrandSearchService()
{
    qCDebug(logGrandSearch) << "GrandSearchService destroyed";
}

bool GrandSearchService::IsVisible() const
{
    Q_ASSERT(d_p->m_mainWindow);

    bool isvisble = d_p->m_mainWindow->isVisible();
    qCDebug(logGrandSearch) << "D-Bus method called: IsVisible - Result:" << isvisble;
    return isvisble;
}

void GrandSearchService::SetVisible(const bool visible)
{
    Q_ASSERT(d_p->m_mainWindow);

    // todo 二阶段：根据情况，可能需要设置为隐藏，而不能直接退出
    if (d_p->m_mainWindow->isVisible() == visible) {
        qCDebug(logGrandSearch) << "Window visibility unchanged:" << visible;
        return;
    }

    if (!visible) {
        qCInfo(logGrandSearch) << "Closing main window via D-Bus SetVisible(false)";
        d_p->m_mainWindow->close();
    } else {
        qCDebug(logGrandSearch) << "Showing main window via D-Bus SetVisible(true)";
        d_p->m_mainWindow->setVisible(visible);
    }
}
