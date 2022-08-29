// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "eventlogutil.h"

#include <QStandardPaths>
#include <QLibrary>
#include <QJsonObject>
#include <QJsonDocument>
#include <QDebug>

using namespace GrandSearch;
using namespace burying_point;

class EventLogUtilGlobal : public EventLogUtil{};
Q_GLOBAL_STATIC(EventLogUtilGlobal, eventLogUtilGlobal)

EventLogUtil::EventLogUtil()
{
    QLibrary library("/usr/lib/libdeepin-event-log.so");

    init = reinterpret_cast<bool (*)(const std::string &, bool)>(library.resolve("Initialize"));
    writeLog = reinterpret_cast<void (*)(const std::string &)>(library.resolve("WriteEventLog"));

    if (init == nullptr || writeLog == nullptr) {
        qWarning() << "Initialize the library failed.";
        return;
    }

    init("dde-grand-search", false);
}

EventLogUtil::~EventLogUtil()
{

}

EventLogUtil *EventLogUtil::instance()
{
    return eventLogUtilGlobal;
}

void EventLogUtil::writeEvent(const QVariantMap &data) const
{
    if (writeLog == nullptr) {
        qWarning() << "WriteEventLog interface call failed";
        return;
    }

    writeLog(QJsonDocument(castToJson(data)).toJson(QJsonDocument::Compact).toStdString());
    qDebug() << "Data has been reported.";
}

QJsonObject EventLogUtil::castToJson(const QVariantMap &data) const
{
    QJsonObject json;
    for (auto it = data.begin(); it != data.end(); ++it) {
        json.insert(it.key(), it.value().toJsonValue());
    }
    return json;
}
