/*
 * Copyright (C) 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     wangchunlin<wangchunlin@uniontech.com>
 *
 * Maintainer: wangchunlin<wangchunlin@uniontech.com>
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
#ifndef GRANDSEARCHSERVICE_P_H
#define GRANDSEARCHSERVICE_P_H

#include "grandsearchservice.h"

class GrandSearchServicePrivate
{
public:
    explicit GrandSearchServicePrivate(MainWindow *mainWindow, GrandSearchService *parent = nullptr);

    GrandSearchService *q_p = nullptr;
    MainWindow *m_mainWindow = nullptr;
};

#endif // GRANDSEARCHSERVICE_P_H
