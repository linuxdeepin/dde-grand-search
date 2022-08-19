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
