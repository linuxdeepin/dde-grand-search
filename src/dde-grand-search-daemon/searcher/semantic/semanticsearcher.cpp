// SPDX-FileCopyrightText: 2023 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "semanticsearcher_p.h"
#include "global/builtinsearch.h"
#include "global/searchconfigdefine.h"
#include "semanticworker.h"
#include "configuration/configer.h"

#include <QLoggingCategory>

Q_DECLARE_LOGGING_CATEGORY(logDaemon)

using namespace GrandSearch;

SemanticSearcherPrivate::SemanticSearcherPrivate(SemanticSearcher *parent)
    : q(parent)
{
}

SemanticSearcherPrivate::~SemanticSearcherPrivate()
{
}

SemanticSearcher::SemanticSearcher(QObject *parent)
    : Searcher(parent)
    , d(new SemanticSearcherPrivate(this))
{
    qCDebug(logDaemon) << "SemanticSearcher constructor";
}

SemanticSearcher::~SemanticSearcher()
{
    delete d;
    d = nullptr;
}

QString SemanticSearcher::name() const
{
    return GRANDSEARCH_CLASS_GENERALFILE_SEMANTIC;
}

bool SemanticSearcher::isActive() const
{
    qCDebug(logDaemon) << "Checking SemanticSearcher active status";

    auto config = ConfigerIns->group(GRANDSEARCH_SEMANTIC_GROUP);
    Q_ASSERT(config);

    // 检查语义分析是否启用
    d->m_semanticEnabled = config->value(GRANDSEARCH_CLASS_GENERALFILE_SEMANTIC_ANALYSIS, true);
    qCDebug(logDaemon) << "Semantic analysis enabled:" << d->m_semanticEnabled;

    // 检查全文搜索是否启用
    d->m_fulltextEnabled = config->value(GRANDSEARCH_CLASS_GENERALFILE_SEMANTIC_FULLTEXT, false);
    qCDebug(logDaemon) << "Fulltext search enabled:" << d->m_fulltextEnabled;

    // 检查 OCR 文本搜索是否启用
    d->m_ocrtextEnabled = config->value(GRANDSEARCH_CLASS_GENERALFILE_SEMANTIC_OCRTEXT, false);
    qCDebug(logDaemon) << "OCR text search enabled:" << d->m_ocrtextEnabled;

    return d->m_semanticEnabled;
}

bool SemanticSearcher::activate()
{
    qCDebug(logDaemon) << "SemanticSearcher activate called - Not supported for this searcher type";
    return false;
}

ProxyWorker *SemanticSearcher::createWorker() const
{
    qCDebug(logDaemon) << "Creating SemanticWorker";

    auto worker = new SemanticWorker(name());
    worker->setEngineState(d->m_semanticEnabled, d->m_ocrtextEnabled, d->m_fulltextEnabled);

    qCInfo(logDaemon) << "Created SemanticWorker - semantic:" << d->m_semanticEnabled
                      << "fulltext:" << d->m_fulltextEnabled
                      << "ocrtext:" << d->m_ocrtextEnabled;

    return worker;
}

bool SemanticSearcher::action(const QString &action, const QString &item)
{
    Q_UNUSED(action)
    Q_UNUSED(item)
    qCWarning(logDaemon) << "Unsupported action requested:" << action;
    return false;
}
