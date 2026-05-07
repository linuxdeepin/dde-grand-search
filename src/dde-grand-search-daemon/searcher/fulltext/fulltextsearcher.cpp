// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "fulltextsearcher.h"
#include "fulltextworker.h"
#include "global/builtinsearch.h"
#include "configuration/configer.h"

#include <dfm-search/dsearch_global.h>

#include <DConfig>

#include <QLoggingCategory>

Q_DECLARE_LOGGING_CATEGORY(logDaemon)

DCORE_USE_NAMESPACE

using namespace GrandSearch;

FullTextSearcher::FullTextSearcher(QObject *parent)
    : Searcher(parent)
{
    qCDebug(logDaemon) << "FullTextSearcher constructor";
}

QString FullTextSearcher::name() const
{
    return GRANDSEARCH_CLASS_FILE_FULLTEXT;
}

bool FullTextSearcher::isActive() const
{
    // Combination search is not supported for full-text search
    // (check from context - if typeKeywords exist, it's combination)
    DConfig *dcfg = DConfig::create("org.deepin.dde.file-manager",
                                    "org.deepin.dde.file-manager.search");
    if (!dcfg)
        return false;

    bool enabled = dcfg->value("enableFullTextSearch", false).toBool();
    delete dcfg;

    if (!enabled) {
        qCDebug(logDaemon) << "FullTextSearcher inactive: full-text search disabled by DConfig";
        return false;
    }

    if (!DFMSEARCH::Global::isContentIndexAvailable()) {
        qCDebug(logDaemon) << "FullTextSearcher inactive: content index not available";
        return false;
    }

    // Check if Document category is enabled
    auto config = ConfigerIns->group(GRANDSEARCH_CLASS_FILE_DEEPIN);
    if (!config->value(GRANDSEARCH_GROUP_FILE_DOCUMNET, false)) {
        qCDebug(logDaemon) << "FullTextSearcher inactive: Document category not enabled";
        return false;
    }

    qCDebug(logDaemon) << "FullTextSearcher active";
    return true;
}

bool FullTextSearcher::activate()
{
    qCDebug(logDaemon) << "FullTextSearcher activate called";
    return isActive();
}

ProxyWorker *FullTextSearcher::createWorker() const
{
    qCDebug(logDaemon) << "Creating FullTextWorker";
    auto worker = new FullTextWorker(name());
    return worker;
}

bool FullTextSearcher::action(const QString &action, const QString &item)
{
    Q_UNUSED(item)
    qCWarning(logDaemon) << "Unsupported action requested - Action:" << action;
    return false;
}
