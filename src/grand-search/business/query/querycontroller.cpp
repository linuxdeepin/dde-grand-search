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
#include "querycontroller_p.h"
#include "querycontroller.h"
#include "contacts/interface/daemongrandsearchinterface.h"
#include "gui/datadefine.h"

#include <QUuid>
#include <QDebug>

using namespace GrandSearch;

static const uint KeepAliveTime             = 15000;    // 搜索过程中，调用后端心跳函数间隔时间

QueryControllerPrivate::QueryControllerPrivate(QueryController *parent)
    : q_p(parent)
{
    m_keepAliveTimer = new QTimer(this);
    m_keepAliveTimer->setInterval(KeepAliveTime);

    m_daemonDbus = new DaemonGrandSearchInterface(this);

    connect(m_keepAliveTimer, &QTimer::timeout, this, &QueryControllerPrivate::keepAlive);
}

void QueryControllerPrivate::keepAlive()
{
    m_daemonDbus->KeepAlive(m_missionId);
}

void QueryControllerPrivate::onSearchCompleted(const QString &missionId)
{
    if (missionId == m_missionId) {
        m_keepAliveTimer->stop();
        qDebug() << "search completed and missionId:" << m_missionId;
    }
}

QueryController::QueryController(QObject *parent)
    : QObject(parent)
    , d_p(new QueryControllerPrivate(this))
{

}

QueryController::~QueryController()
{

}

void QueryController::onSearchTextChanged(const QString &txt)
{
    if (txt == d_p->m_searchText)
        return;

    d_p->m_keepAliveTimer->stop();

    if (txt.isEmpty()) {
        onTerminateSearch();
        qDebug() << "search terminate and missionId:" << d_p->m_missionId;
        d_p->m_searchText.clear();
        d_p->m_missionId.clear();

        // 发出搜索文本为空信号
        emit searchTextIsEmpty();

        return;
    }

    d_p->m_searchText = txt;

    // 搜索文本改变，创建新的会话ID
    d_p->m_missionId = QUuid::createUuid().toString(QUuid::WithoutBraces);
    emit missionIdChanged(d_p->m_missionId);

    qDebug() << QString("m_daemonDbus->Search begin missionId:%1").arg(d_p->m_missionId);
    bool started = d_p->m_daemonDbus->Search(d_p->m_missionId, d_p->m_searchText);
    qDebug() << QString("m_daemonDbus->Search end   missionId:%1").arg(d_p->m_missionId);
    if (started) {
        d_p->m_keepAliveTimer->start();
    } else {
        qWarning() << QString("search failed missionId:%1").arg(d_p->m_missionId);
    }
}

void QueryController::onTerminateSearch()
{
    if (d_p->m_missionId.isEmpty())
        return;
    qDebug() << "m_daemonDbus->Terminate begin missionId:" << d_p->m_missionId;
    d_p->m_daemonDbus->Terminate();
    qDebug() << "m_daemonDbus->Terminate end   missionId:" << d_p->m_missionId;
}

QString QueryController::getMissionID() const
{
    return d_p->m_missionId;
}

bool QueryController::isEmptySearchText() const
{
    return d_p->m_searchText.isEmpty();
}
