// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef BASICPOINT_H
#define BASICPOINT_H

#include <QPair>
#include <QVariantMap>

namespace GrandSearch {

namespace burying_point {

enum BuryingPointEventId {
    // Each event has its unique id defined by data platform except Default id.
    Default = 0,  // default event id, which is only used to identify base data
    Launch = 1000000003,  // launch application event id
    ClickItem = 1000700000  // click the search result event id
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
