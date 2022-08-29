// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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
