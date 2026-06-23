// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SEMANTICWORKER_P_H
#define SEMANTICWORKER_P_H

#include "semanticworker.h"
#include "searcher/file/filesearchutils.h"

#include <dfm-search/semanticsearcher.h>
#include <dfm-search/semantic_types.h>

#include <QMutex>
#include <QAtomicInt>
#include <QElapsedTimer>
#include <QSet>

namespace GrandSearch {

class SemanticWorkerPrivate
{
public:
    explicit SemanticWorkerPrivate(SemanticWorker *qq);
    ~SemanticWorkerPrivate();

    bool doSearch();
    void processResults(const DFMSEARCH::SearchResultList &results);
    void tryNotify();
    int itemCount() const;

public:
    SemanticWorker *q_ptr = nullptr;
    QAtomicInt m_status = ProxyWorker::Ready;
    QString m_context;                      // 搜索上下文（自然语言查询）
    QStringList m_keywords;                 // 从 intentParsed 提取的关键词

    mutable QMutex m_mutex;
    MatchedItems m_items;
    QSet<QString> m_tmpSearchResults;       // 用于去重

    QElapsedTimer m_time;
    int m_lastEmit = 0;

    Q_DECLARE_PUBLIC(SemanticWorker)
};

}

#endif // SEMANTICWORKER_P_H
