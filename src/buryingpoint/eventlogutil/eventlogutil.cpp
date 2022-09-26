// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "eventlogutil.h"
#include "committhread.h"

#include <QJsonObject>
#include <QJsonDocument>
#include <QThread>
#include <QDebug>

using namespace GrandSearch;
using namespace burying_point;

class EventLogUtilGlobal : public EventLogUtil
{
};
Q_GLOBAL_STATIC(EventLogUtilGlobal, eventLogUtilGlobal)

EventLogUtil::EventLogUtil(QObject *parent)
    : QObject(parent)
{
    init();
}

EventLogUtil::~EventLogUtil()
{
    if (m_commitThread) {
        m_commitThread->quit();
        m_commitThread->wait(2000);
    }
}

EventLogUtil *EventLogUtil::instance()
{
    return eventLogUtilGlobal;
}

void EventLogUtil::writeEvent(const QVariantMap &data)
{
    if (!m_isInit) {
        qWarning() << "EventLogUtil is not init";
        return;
    }

    emit appendData(QJsonDocument(castToJson(data)).toJson(QJsonDocument::Compact));
}

QJsonObject EventLogUtil::castToJson(const QVariantMap &data) const
{
    QJsonObject json;
    for (auto it = data.begin(); it != data.end(); ++it) {
        json.insert(it.key(), it.value().toJsonValue());
    }
    return json;
}

void EventLogUtil::init()
{
    if (m_isInit)
        return;

    m_commitLog = new CommitLog();
    if (!m_commitLog->init())
        return;

    m_commitThread = new QThread();
    connect(this, &EventLogUtil::appendData, m_commitLog, &CommitLog::commit);
    connect(m_commitThread, &QThread::finished, [&]() {
        m_commitLog->deleteLater();
    });
    m_commitLog->moveToThread(m_commitThread);
    m_commitThread->start();

    m_isInit = true;
}
