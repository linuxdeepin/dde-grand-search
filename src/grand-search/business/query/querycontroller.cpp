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
#include "gui/mainwindow.h"
#include "contacts/interface/daemongrandsearchinterface.h"

#include <QUuid>
#include <QDebug>

class QueryControllerGlobal : public QueryController{};
Q_GLOBAL_STATIC(QueryControllerGlobal, queryControllerGlobal)

QueryControllerPrivate::QueryControllerPrivate(QueryController *parent)
    : q_p(parent)
{
    m_keepAliveTimer = new QTimer(this);
    m_keepAliveTimer->setInterval(3000);

    m_daemonDbus = new DaemonGrandSearchInterface(this);

    connect(m_keepAliveTimer, &QTimer::timeout, this, &QueryControllerPrivate::keepAlive);
}

void QueryControllerPrivate::onSearchTextChanged(const QString &txt)
{
    if (m_searchText == txt)
        return;

    m_keepAliveTimer->stop();

    if (txt.isEmpty()) {
        m_daemonDbus->Terminate();
        qDebug() << "search terminate and missionId:" << m_missionId;
        m_searchText.clear();
        m_missionId.clear();
        return;
    }

    m_searchText = txt;

    // 搜索文本改变，创建新的会话ID
    m_missionId = QUuid::createUuid().toString(QUuid::WithoutBraces);
    emit q_p->missionIdChanged(m_missionId);

    qDebug() << QString("search started and missionId:%1 searchText:%2").arg(m_missionId).arg(m_searchText);
    bool started = m_daemonDbus->Search(m_missionId, m_searchText);
    if (started) {
        m_keepAliveTimer->start();
    } else {
        qWarning() << QString("search failed missionId:%1 searchText:%2").arg(m_missionId).arg(m_searchText);
    }
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
    connect(MainWindow::instance(), &MainWindow::searchTextChanged, d_p.data(), &QueryControllerPrivate::onSearchTextChanged);
}

QueryController::~QueryController()
{

}

QString QueryController::getMissionID() const
{
    return d_p->m_missionId;
}

QueryController *QueryController::instance()
{
    return queryControllerGlobal;
}


