// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "statictextechoer.h"
#include "global/builtinsearch.h"
#include "statictextworker.h"

#include <QDebug>
#include <QLoggingCategory>

Q_DECLARE_LOGGING_CATEGORY(logDaemon)

using namespace GrandSearch;

StaticTextEchoer::StaticTextEchoer(QObject *parent)
    : Searcher(parent)
{
    qCDebug(logDaemon) << "StaticTextEchoer constructor";
}

QString StaticTextEchoer::name() const
{
    return GRANDSEARCH_CLASS_WEB_STATICTEXT;
}

bool StaticTextEchoer::isActive() const
{
    return true;
}

bool StaticTextEchoer::activate()
{
    qCDebug(logDaemon) << "StaticTextEchoer activate called - Not supported for this searcher type";
    return false;
}

ProxyWorker *StaticTextEchoer::createWorker() const
{
    qCDebug(logDaemon) << "Creating StaticTextWorker";
    return new StaticTextWorker(name());
}

bool StaticTextEchoer::action(const QString &action, const QString &item)
{
    Q_UNUSED(item);
    qCWarning(logDaemon) << "Unsupported action requested:" << action;
    return false;
}
