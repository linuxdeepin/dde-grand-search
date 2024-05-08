// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef FULLTEXTQUERY_P_H
#define FULLTEXTQUERY_P_H

#include "fulltextquery.h"
#include "searcher/semantic/fileresultshandler.h"

namespace GrandSearch {

class FullTextQueryPrivate
{
public:
    struct Context
    {
       FullTextQuery *query;
       PushItemCallBack callBack;
       void *callBackData;
       QStringList suffix;
       QList<QPair<qint64, qint64>> times;
    };
public:
    explicit FullTextQueryPrivate(FullTextQuery *qq);
    static inline QString indexStorePath() {
        static QString path = QStandardPaths::standardLocations(QStandardPaths::ConfigLocation).first()
                + "/deepin/dde-file-manager/index";
        return path;
    }
    static bool processResult(const QString &file, void *pdata);
    bool timeToPush() const;
public:
    SemanticEntity m_entity;
    FileResultsHandler *m_handler = nullptr;
    QTime m_time;
    int m_lastPush = 0;
    MatchedItemMap m_results;
private:
    FullTextQuery *q;
};

}


#endif // FULLTEXTQUERY_P_H
