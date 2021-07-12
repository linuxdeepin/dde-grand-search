/*
 * Copyright (C) 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     wangchunlin<wangchunlin@uniontech.com>
 *
 * Maintainer: wangchunlin<wangchunlin@uniontech.com>
 *             houchengqiu<houchengqiu@uniontech.com>
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
#include "matchcontroller_p.h"
#include "matchcontroller.h"

class MatchControllerGlobal : public MatchController {};
Q_GLOBAL_STATIC(MatchControllerGlobal, matchControllerGlobal)

MatchControllerPrivate::MatchControllerPrivate(MatchController *parent)
    : q_p(parent)
{

}

MatchController::MatchController(QObject *parent)
    : QObject(parent)
    , d_p(new MatchControllerPrivate(this))
{

}

MatchController::~MatchController()
{

}

MatchController *MatchController::instance()
{
    return matchControllerGlobal;
}

