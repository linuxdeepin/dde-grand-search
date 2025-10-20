// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "filenamesearcher.h"
#include "global/builtinsearch.h"
#include "filenameworker.h"

#include <QDBusInterface>
#include <QDBusReply>
#include <QDBusConnectionInterface>
#include <QLoggingCategory>

Q_DECLARE_LOGGING_CATEGORY(logDaemon)

using namespace GrandSearch;

FileNameSearcher::FileNameSearcher(QObject *parent)
    : Searcher(parent)
{
    qCDebug(logDaemon) << "FileNameSearcher constructor";
}

QString FileNameSearcher::name() const
{
    return GRANDSEARCH_CLASS_FILE_DEEPIN;
}

bool FileNameSearcher::isActive() const
{
    qCDebug(logDaemon) << "FileNameSearcher activity check - Always active: true";
    return true;
}

bool FileNameSearcher::activate()
{
    qCDebug(logDaemon) << "FileNameSearcher activate called - Not supported for this searcher type";
    return false;
}

ProxyWorker *FileNameSearcher::createWorker() const
{
    qCDebug(logDaemon) << "Creating FileNameWorker";
    auto worker = new FileNameWorker(name());
    return worker;
}

bool FileNameSearcher::action(const QString &action, const QString &item)
{
    Q_UNUSED(item)
    qCWarning(logDaemon) << "Unsupported action requested - Action:" << action;
    return false;
}
