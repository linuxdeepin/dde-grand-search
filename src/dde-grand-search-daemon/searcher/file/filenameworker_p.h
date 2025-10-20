// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef FILENAMEWORKER_P_H
#define FILENAMEWORKER_P_H

#include "filenameworker.h"
#include "filesearchutils.h"

#include <dfm-search/searchfactory.h>
#include <dfm-search/filenamesearchapi.h>

namespace GrandSearch {

class FileNameWorkerPrivate
{
public:
    explicit FileNameWorkerPrivate(FileNameWorker *qq);
    void initConfig();
    bool appendSearchResult(const QString &fileName);
    bool searchByDFMSearch();

    DFMSEARCH::SearchOptions createSearchOptions() const;
    DFMSEARCH::SearchQuery createSearchQuery() const;
    bool processSearchResults(const DFMSEARCH::SearchResultExpected &result);
    void configureFileNameOptions(DFMSEARCH::FileNameOptionsAPI &fileNameOptions, const DFMSEARCH::SearchQuery &query) const;

    void tryNotify();
    int itemCount() const;
    // 判断所有类目的搜索结果是否达到限制数量
    bool isResultLimit();

public:
    FileNameWorker *q_ptr = nullptr;
    QAtomicInt m_status = ProxyWorker::Ready;
    QString m_searchPath;
    FileSearchUtils::SearchInfo m_searchInfo;   // 搜索信息
    QHash<FileSearchUtils::Group, quint32> m_resultCountHash;   // 记录各类型文件搜索结果数量

    mutable QMutex m_mutex;
    MatchedItems m_items[FileSearchUtils::GroupCount];   // 文件搜索
    QSet<QString> m_tmpSearchResults;   // 存储所有的搜索结果，用于去重

    // 计时
    QElapsedTimer m_time;
    int m_lastEmit = 0;

    Q_DECLARE_PUBLIC(FileNameWorker)
};

}

#endif   // FILENAMEWORKER_P_H
