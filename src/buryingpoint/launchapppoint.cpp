// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "launchapppoint.h"

#include <QVariantMap>

using namespace GrandSearch;
using namespace burying_point;

LaunchAppPoint::LaunchAppPoint()
    : m_version(QPair<QString, QString>("version", QCoreApplication::applicationVersion())),
      m_mode(QPair<QString, int>("mode", 1))
{
    m_eventId = QPair<QString, BuryingPointEventId>("tid", Launch);
}

LaunchAppPoint::~LaunchAppPoint()
{

}

QVariantMap LaunchAppPoint::assemblingData() const
{
    QVariantMap map;
    map.insert(m_eventId.first, m_eventId.second);
    map.insert(m_version.first, m_version.second);
    map.insert(m_mode.first, m_mode.second);
    return map;
}
