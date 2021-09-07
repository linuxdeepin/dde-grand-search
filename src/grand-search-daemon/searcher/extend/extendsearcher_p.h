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
#ifndef EXTENDSEARCHER_P_H
#define EXTENDSEARCHER_P_H

#include "extendsearcher.h"

class ExtendSearcherPrivate : public QObject
{
    Q_OBJECT
    friend class ExtendSearcher;
public:
    ExtendSearcherPrivate(ExtendSearcher *parent);
private:
    QString m_name;
    QString m_version;
    bool m_activatable = false;

    QString m_service;    //dbus服务
    QString m_address;    //dbus地址
    QString m_interface;  //dbus接口
};

#endif // EXTENDSEARCHER_P_H
