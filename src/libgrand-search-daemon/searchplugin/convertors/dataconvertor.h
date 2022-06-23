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
#ifndef DATACONVERTOR_H
#define DATACONVERTOR_H

#include "abstractconvertor.h"

#include <QtCore>

#define DataConvIns GrandSearch::DataConvertor::instance()

namespace GrandSearch {

typedef AbstractConvertor* (*CreateConvertor)();

class DataConvertor
{
public:
    static DataConvertor *instance();
    void initConvetor();
    bool isSupported(const QString &ver);
    int convert(const QString &version, const QString &type, void *in, void *out);
    bool regist(const QString &ver, CreateConvertor creator);
    void unRegist(CreateConvertor creator);

protected:
    explicit DataConvertor();
protected:
    bool m_inited = false;
    QHash<QString, CreateConvertor> m_convertors;
    QReadWriteLock m_rwLock;
};

}

#endif // DATACONVERTOR_H
