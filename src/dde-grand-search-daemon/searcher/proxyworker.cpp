// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "proxyworker.h"
#include <QLoggingCategory>

Q_DECLARE_LOGGING_CATEGORY(logDaemon)

using namespace GrandSearch;

ProxyWorker::ProxyWorker(const QString &name, QObject *parent)
    : QObject(parent)
    , m_name(name)
{
    Q_ASSERT(!name.isEmpty());
    qCDebug(logDaemon) << "ProxyWorker constructor - Name:" << name;
}

QString ProxyWorker::name() const
{
    return m_name;
}
