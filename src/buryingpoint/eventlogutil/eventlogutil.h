// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef EVENTLOGUTIL_H
#define EVENTLOGUTIL_H

#include <QVariantMap>

namespace  GrandSearch {

namespace burying_point {

class EventLogUtil
{
public:
    ~EventLogUtil();

    static EventLogUtil *instance();
    void writeEvent(const QVariantMap &data) const;

protected:
    explicit EventLogUtil();
private:
    QJsonObject castToJson(const QVariantMap &data) const;

private:
    bool (*init)(const std::string &packgename, bool enable_sig) = nullptr;
    void (*writeLog)(const std::string &eventData) = nullptr;
};

}



}

#endif // EVENTLOGUTIL_H
