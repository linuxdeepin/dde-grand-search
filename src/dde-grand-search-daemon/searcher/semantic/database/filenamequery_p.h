// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef FILENAMEQUERY_P_H
#define FILENAMEQUERY_P_H

#include "filenamequery.h"
#include "searcher/semantic/fileresultshandler.h"

#include <dfm-search/searchfactory.h>
#include <dfm-search/filenamesearchapi.h>

namespace GrandSearch {

class FileNameQueryPrivate
{
public:
    explicit FileNameQueryPrivate();
    void searchByDFMSearch(PushItemCallBack callBack, void *pdata);

    DFMSEARCH::SearchOptions createSearchOptions(const SemanticEntity &entity) const;
    DFMSEARCH::SearchQuery createSearchQuery(const SemanticEntity &entity) const;
    void configureFileNameOptions(DFMSEARCH::FileNameOptionsAPI &fileNameOptions, const DFMSEARCH::SearchQuery &query, const SemanticEntity &entity) const;
    bool processSearchResults(const DFMSEARCH::SearchResultExpected &result, const SemanticEntity &entity, PushItemCallBack callBack, void *pdata);
    bool checkResultValid(const QString &result, const SemanticEntity &entity);

    bool timeToPush() const;
    double calcItemWeight(const QString &name);

public:
    MatchedItemMap m_resultItems;
    QList<SemanticEntity> m_entity;

    FileResultsHandler *m_handler = nullptr;
    QElapsedTimer m_time;
    int m_lastPush = 0;
    int m_count = 0;
};
}

#endif   // FILENAMEQUERY_P_H
