// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "basicpoint.h"

using namespace GrandSearch;
using namespace burying_point;

BasicPoint::BasicPoint()
    : m_eventId(QPair<QString, BuryingPointEventId>("tid", BuryingPointEventId::Default))
{

}

BasicPoint::~BasicPoint()
{

}

QVariantMap BasicPoint::assemblingData() const
{
    QVariantMap map;
    map.insert(m_eventId.first, m_eventId.second);

    if (!m_additionalData.isEmpty()) {
        for (auto it = m_additionalData.begin(); it != m_additionalData.end(); ++it) {
            map.insert(it.key(), it.value());
        }
    }

    return map;
}

void BasicPoint::setAdditionalData(const QVariantMap &data)
{
    m_additionalData = data;
}
