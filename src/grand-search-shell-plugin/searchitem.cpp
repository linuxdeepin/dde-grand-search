// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "searchitem.h"

#include <pluginfactory.h>

#include <DDBusSender>
#include <DDciIcon>
#include <DGuiApplicationHelper>

#include <QBuffer>
#include <QGuiApplication>
#include <QProcess>

DGUI_USE_NAMESPACE
namespace dock
{

const QString grandSearchService = "com.deepin.dde.GrandSearch";
const QString grandSearchPath = "/com/deepin/dde/GrandSearch";
const QString grandSearchInterface = "com.deepin.dde.GrandSearch";
static DDBusSender searchDbus()
{
    return DDBusSender().service(grandSearchService).path(grandSearchPath).interface(grandSearchInterface);
}

SearchItem::SearchItem(QObject *parent)
    : DApplet(parent)
    , m_visible(true)
    , m_grandSearchVisible(false)
{
}

void SearchItem::toggleGrandSearch()
{
    searchDbus().method("SetVisible").arg(true).call();
}

void SearchItem::toggleGrandSearchConfig()
{
    QProcess::startDetached("dde-grand-search", QStringList() << "--setting");
}

void SearchItem::setVisible(bool visible)
{
    if (m_visible != visible) {
        m_visible = visible;

        Q_EMIT visibleChanged(visible);
    }
}

void SearchItem::onGrandSearchVisibleChanged(bool visible)
{
    if (m_grandSearchVisible != visible) {
        m_grandSearchVisible = visible;

        Q_EMIT grandSearchVisibleChanged(visible);
    }
}

D_APPLET_CLASS(SearchItem)
}

#include "searchitem.moc"
