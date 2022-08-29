// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef BURYINGPOINTFACTORY_H
#define BURYINGPOINTFACTORY_H

#include "basicpoint.h"

namespace GrandSearch {

namespace burying_point {

class BuryingPointFactory
{
public:
    static BuryingPointFactory *instance();
    ~BuryingPointFactory();

    BasicPoint *createData(const BuryingPointEventId &id) const;

protected:
    explicit BuryingPointFactory();
};

}

}

#endif // BURYINGPOINTFACTORY_H
