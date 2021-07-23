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
#include "dataconvertor.h"
#include "convertors/convertorv1_0.h"

class DataConvertorGlobal : public DataConvertor {};
Q_GLOBAL_STATIC(DataConvertorGlobal, dataConvertorGlobal)

DataConvertor *DataConvertor::instance()
{
    return dataConvertorGlobal;
}

void DataConvertor::initConvetor()
{
    if (m_inited)
        return;
    m_inited = true;

    //注册1.0版本
    {
        ConvertorV1_0 v1;
        regist(v1.version(), ConvertorV1_0::create);
    }
}

bool DataConvertor::isSupported(const QString &ver)
{
    QReadLocker lk(&m_rwLock);
    return m_convertors.contains(ver);
}

int DataConvertor::convert(const QString &version, const QString &type, QJsonObject *json, void *info)
{
    if (version.isEmpty() || type.isEmpty() || json == nullptr || info == nullptr)
        return -1;

    QReadLocker lk(&m_rwLock);
    if (auto creator = m_convertors.value(version)) {
        QSharedPointer<AbstractConvertor> converter((*creator)()) ;
        Q_ASSERT(converter);

        auto ifs = converter->interfaces();
        if (ifs.contains(type)) {
            auto func = ifs.value(type);
            Q_ASSERT(func);

            return (*func)(json, info);
        }
    }

    return 1;
}

bool DataConvertor::regist(const QString &ver, CreateConvertor creator)
{
    Q_ASSERT(creator);

    QWriteLocker lk(&m_rwLock);
    if (m_convertors.contains(ver))
        return false;
    m_convertors.insert(ver, creator);

    return true;
}

void DataConvertor::unRegist(CreateConvertor creator)
{
    QWriteLocker lk(&m_rwLock);
    auto key = m_convertors.key(creator);
    if (!key.isEmpty()) {
        m_convertors.remove(key);
    }
}

DataConvertor::DataConvertor()
{

}
