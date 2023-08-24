// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef FEATUREQUERY_P_H
#define FEATUREQUERY_P_H

#include "featurequery.h"
#include "luceneengine/featurelibengine.h"

namespace GrandSearch {

class FeatureQueryPrivate
{
public:
    struct Context
    {
       FeatureQuery *query;
       PushItemCallBack callBack;
       void *callBackData;
    };
public:
    explicit FeatureQueryPrivate(FeatureQuery *qq);
    static inline QString indexStorePath() {
        static QString path = QStandardPaths::standardLocations(QStandardPaths::GenericCacheLocation).first()
                + "/deepin/deepin-ai-daemon/index";
        return path;
    }
    static bool processResult(const QString &file, void *pdata);
    FeatureLibEngine::QueryConditons translateConditons();
    bool timeToPush() const;
public:
    SemanticEntity m_entity;
    FileResultsHandler *m_handler = nullptr;
    QTime m_time;
    int m_lastPush = 0;
    MatchedItemMap m_results;
private:
    FeatureQuery *q;
};

}


#endif // FEATUREQUERY_P_H
