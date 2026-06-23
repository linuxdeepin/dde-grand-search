// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "semanticsearcher.h"
#include "semanticworker.h"
#include "global/builtinsearch.h"
#include "configuration/configer.h"

#include <dfm-search/dsearch_global.h>

#include <QLoggingCategory>

Q_DECLARE_LOGGING_CATEGORY(logDaemon)

DFM_SEARCH_USE_NS

using namespace GrandSearch;

SemanticSearcher::SemanticSearcher(QObject *parent)
    : Searcher(parent)
{
    qCDebug(logDaemon) << "SemanticSearcher constructor";
}

QString SemanticSearcher::name() const
{
    return GRANDSEARCH_CLASS_FILE_SEMANTIC;
}

bool SemanticSearcher::isActive() const
{
    if (!Global::isFileNameIndexReadyForSearch()) {
        qCDebug(logDaemon) << "SemanticSearcher inactive: filename index not ready";
        return false;
    }

    auto config = ConfigerIns->group(GRANDSEARCH_SEMANTIC_GROUP);
    if (!config->value(GRANDSEARCH_SEMANTIC_ENABLED, true)) {
        qCDebug(logDaemon) << "SemanticSearcher inactive: semantic search disabled in configuration";
        return false;
    }

    qCDebug(logDaemon) << "SemanticSearcher active";
    return true;
}

ProxyWorker *SemanticSearcher::createWorker() const
{
    qCDebug(logDaemon) << "Creating SemanticWorker";
    auto worker = new SemanticWorker(name());
    return worker;
}

bool SemanticSearcher::action(const QString &action, const QString &item)
{
    Q_UNUSED(item)
    qCWarning(logDaemon) << "Unsupported action requested - Action:" << action;
    return false;
}
