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
#ifndef BASICPOINT_H
#define BASICPOINT_H

#include <QPair>
#include <QVariantMap>

namespace GrandSearch {

namespace burying_point {

enum BuryingPointEventId {
    // Each event has its unique id defined by data platform except Default id.
    Default = 0,  // default event id, which is only used to identify base data
    Launch = 1000000003  // launch application event id
};

class BasicPoint
{
public:
    explicit BasicPoint();
    virtual ~BasicPoint();

    virtual QVariantMap assemblingData() const;
    virtual void setAdditionalData(const QVariantMap &data);

protected:
    QPair<QString, BuryingPointEventId> m_eventId;
    QVariantMap m_additionalData;
};

}

}

#endif // BASICPOINT_H
