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
