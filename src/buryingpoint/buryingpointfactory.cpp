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
#include "buryingpointfactory.h"
#include "launchapppoint.h"

using namespace GrandSearch;
using namespace burying_point;

class BuryingPointDataFactoryGlobal : public BuryingPointFactory{};
Q_GLOBAL_STATIC(BuryingPointDataFactoryGlobal, buryingPointDataFactoryGlobal)

BuryingPointFactory::BuryingPointFactory()
{

}

BuryingPointFactory *BuryingPointFactory::instance()
{
    return buryingPointDataFactoryGlobal;
}

BuryingPointFactory::~BuryingPointFactory()
{

}

BasicPoint *BuryingPointFactory::createData(const GrandSearch::burying_point::BuryingPointEventId &id) const
{
    BasicPoint *p = nullptr;
    switch (id) {
    case BuryingPointEventId::Launch: {
        p = new LaunchAppPoint();
        break;
    }
    default:
        p = new BasicPoint();
    }

    return p;
}
