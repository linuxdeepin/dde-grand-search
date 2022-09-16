// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "clickitempoint.h"

#include <QCoreApplication>
#include <QVariantMap>

using namespace GrandSearch;
using namespace burying_point;

ClickItemPoint::ClickItemPoint()
    : m_version(QPair<QString, QString>("version", QCoreApplication::applicationVersion()))
{
    m_eventId = QPair<QString, BuryingPointEventId>("tid", ClickItem);
}

ClickItemPoint::~ClickItemPoint()
{

}

QVariantMap ClickItemPoint::assemblingData() const
{
    QVariantMap map;
    map.insert(m_eventId.first, m_eventId.second);
    map.insert(m_version.first, m_version.second);
    for (auto it = m_additionalData.begin(); it != m_additionalData.end(); ++it) {
        map.insert(it.key(), it.value());
    }

    return map;
}
