/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhaohanxi<zhaohanxi@uniontech.com>
 *
 * Maintainer: zhaohanxi<zhaohanxi@uniontech.com>
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
