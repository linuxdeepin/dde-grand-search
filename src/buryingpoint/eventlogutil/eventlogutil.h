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
