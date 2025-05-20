// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "querycontroller_p.h"
#include "querycontroller.h"
#include "gui/datadefine.h"

#include "interfaces/daemongrandsearchinterface.h"

#include <QUuid>
#include <QDebug>

using namespace GrandSearch;

static const uint KeepAliveTime  = 15000;    // 搜索过程中，调用后端心跳函数间隔时间

QueryControllerPrivate::QueryControllerPrivate(QueryController *parent)
    : q_p(parent)
{
    m_keepAliveTimer = new QTimer(this);
    m_keepAliveTimer->setInterval(KeepAliveTime);

    // 初始化防抖定时器
    m_debounceTimer = new QTimer(this);
    m_debounceTimer->setSingleShot(true);
    m_debounceTimer->setInterval(100);

    m_daemonDbus = new DaemonGrandSearchInterface(this);

    // 前端启动时，通知后端停止可能的已有搜索任务，同时通过DBus调用保证后端已经启动
    m_daemonDbus->Terminate();

    connect(m_keepAliveTimer, &QTimer::timeout, this, &QueryControllerPrivate::keepAlive);
    connect(m_debounceTimer, &QTimer::timeout, this, &QueryControllerPrivate::performSearch);
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

void QueryControllerPrivate::performSearch()
{
    // 如果待处理文本为空，直接返回
    if (m_pendingSearchText.isEmpty())
        return;

    // 更新当前搜索文本
    m_searchText = m_pendingSearchText;
    m_pendingSearchText.clear();

    // 搜索文本改变，创建新的会话ID
    m_missionId = QUuid::createUuid().toString(QUuid::WithoutBraces);
    emit q_p->missionChanged(m_missionId, m_searchText);

    qDebug() << QString("m_daemonDbus->Search begin missionId:%1").arg(m_missionId);
    bool started = m_daemonDbus->Search(m_missionId, m_searchText);
    qDebug() << QString("m_daemonDbus->Search end   missionId:%1").arg(m_missionId);
    if (started) {
        m_keepAliveTimer->start();
    } else {
        qWarning() << QString("search failed missionId:%1").arg(m_missionId);
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
    // 如果文本没有变化，直接返回
    if (txt == d_p->m_searchText && txt == d_p->m_pendingSearchText)
        return;

    // 停止已有的定时器
    d_p->m_debounceTimer->stop();

    if (txt.isEmpty()) {
         // 停止搜索相关的所有活动
        onTerminateSearch();
        qDebug() << "search terminate and missionId:" << d_p->m_missionId;
        d_p->m_searchText.clear();
        d_p->m_pendingSearchText.clear();
        d_p->m_missionId.clear();
        d_p->m_keepAliveTimer->stop();

        // 发出搜索文本为空信号
        emit missionChanged(d_p->m_missionId, d_p->m_searchText);

        return;
    }

    // 存储待处理的搜索文本
    d_p->m_pendingSearchText = txt;

    // 设置防抖延迟，根据文本长度动态调整
    int debounceDelay = 0;
    if (txt.length() <= 2) {
        debounceDelay = 300;   // 短文本使用更长的延迟
    } else {
        debounceDelay = 0;
    }

    d_p->m_debounceTimer->setInterval(debounceDelay);
    d_p->m_debounceTimer->start();
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
