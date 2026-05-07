// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef FULLTEXTWORKER_P_H
#define FULLTEXTWORKER_P_H

#include "fulltextworker.h"
#include "searcher/file/filesearchutils.h"

#include <dfm-search/searchfactory.h>
#include <dfm-search/contentsearchapi.h>

namespace GrandSearch {

class FullTextWorkerPrivate
{
public:
    explicit FullTextWorkerPrivate(FullTextWorker *qq);
    bool doSearch();
    bool processResults(const DFMSEARCH::SearchResultExpected &result);

    void tryNotify();
    int itemCount() const;

public:
    FullTextWorker *q_ptr = nullptr;
    QAtomicInt m_status = ProxyWorker::Ready;
    QString m_searchPath;
    FileSearchUtils::SearchInfo m_searchInfo;

    mutable QMutex m_mutex;
    MatchedItems m_items[FileSearchUtils::GroupCount];
    QSet<QString> m_tmpSearchResults;
    bool m_fileGroupEnabled = false;

    QElapsedTimer m_time;
    int m_lastEmit = 0;

    Q_DECLARE_PUBLIC(FullTextWorker)
};

}

#endif   // FULLTEXTWORKER_P_H
