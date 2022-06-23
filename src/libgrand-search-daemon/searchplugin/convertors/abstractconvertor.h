/*
 * Copyright (C) 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhangyu<zhangyub@uniontech.com>
 *
 * Maintainer: zhangyu<zhangyub@uniontech.com>
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
#ifndef ABSTRACTCONVERTOR_H
#define ABSTRACTCONVERTOR_H

#include <QJsonObject>
#include <QHash>

//interface
#define PLUGININTERFACE_TYPE_SEARCH "search"
#define PLUGININTERFACE_TYPE_RESULT "result"
#define PLUGININTERFACE_TYPE_STOP "stop"
#define PLUGININTERFACE_TYPE_ACTION "action"

namespace GrandSearch {

typedef int (*ConvertInterface)(void *, void *);

class AbstractConvertor
{
public:
    explicit AbstractConvertor();
    virtual ~AbstractConvertor();
    virtual QString version() const;
    virtual QHash<QString, ConvertInterface> interfaces();
};

}

#endif // ABSTRACTCONVERTOR_H
