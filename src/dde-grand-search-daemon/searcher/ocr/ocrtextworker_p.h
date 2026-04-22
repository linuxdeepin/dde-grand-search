// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef OCRTEXTWORKER_P_H
#define OCRTEXTWORKER_P_H

#include "ocrtextworker.h"

#include <dfm-search/searchfactory.h>
#include <dfm-search/ocrtextsearchapi.h>

#include <QSet>

namespace GrandSearch {

class OcrTextWorkerPrivate
{
public:
    explicit OcrTextWorkerPrivate(OcrTextWorker *qq);
    bool searchByDFMSearch();

    DFMSEARCH::SearchOptions createSearchOptions() const;
    DFMSEARCH::SearchQuery createSearchQuery() const;
    bool processSearchResults(const DFMSEARCH::SearchResultExpected &result);

    void tryNotify();
    int itemCount() const;
    bool isResultLimit();

public:
    OcrTextWorker *q_ptr = nullptr;
    QAtomicInt m_status = ProxyWorker::Ready;
    QString m_keyword;   // Search keyword

    mutable QMutex m_mutex;
    MatchedItems m_items;   // OCR search results
    QSet<QString> m_tmpSearchResults;   // For deduplication

    quint32 m_resultCount = 0;   // Result count

    // Timing
    QElapsedTimer m_time;
    int m_lastEmit = 0;

    Q_DECLARE_PUBLIC(OcrTextWorker)
};

}

#endif   // OCRTEXTWORKER_P_H
