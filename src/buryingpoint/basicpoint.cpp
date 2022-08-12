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
