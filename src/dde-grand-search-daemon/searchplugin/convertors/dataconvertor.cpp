// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dataconvertor.h"
#include "convertorv1_0.h"

#include <QLoggingCategory>

Q_DECLARE_LOGGING_CATEGORY(logDaemon)

using namespace GrandSearch;

class DataConvertorGlobal : public DataConvertor {};
Q_GLOBAL_STATIC(DataConvertorGlobal, dataConvertorGlobal)

DataConvertor *DataConvertor::instance()
{
    return dataConvertorGlobal;
}

void DataConvertor::initConvetor()
{
    if (m_inited) {
        qCDebug(logDaemon) << "DataConvertor already initialized";
        return;
    }

    qCDebug(logDaemon) << "Initializing data convertor";
    m_inited = true;

    //注册1.0版本
    {
        ConvertorV1_0 v1;
        regist(v1.version(), &ConvertorV1_0::create);
        qCDebug(logDaemon) << "Registered convertor version:" << v1.version();
    }
}

bool DataConvertor::isSupported(const QString &ver)
{
    QReadLocker lk(&m_rwLock);
    return m_convertors.contains(ver);
}

int DataConvertor::convert(const QString &version, const QString &type, void *in, void *out)
{
    if (version.isEmpty() || type.isEmpty() || in == nullptr || out == nullptr) {
        qCWarning(logDaemon) << "Invalid conversion parameters - Version:" << version
                             << "Type:" << type << "Input:" << (in != nullptr)
                             << "Output:" << (out != nullptr);
        return -1;
    }

    QReadLocker lk(&m_rwLock);
    if (auto creator = m_convertors.value(version)) {
        QSharedPointer<AbstractConvertor> converter((*creator)()) ;
        Q_ASSERT(converter);

        auto ifs = converter->interfaces();
        if (ifs.contains(type)) {
            auto func = ifs.value(type);
            Q_ASSERT(func);

            return (*func)(in, out);
        }
    } else {
        qCWarning(logDaemon) << "Unsupported convertor version:" << version;
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
