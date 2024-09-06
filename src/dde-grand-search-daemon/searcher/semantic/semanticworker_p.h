// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SEMANTICWORKER_P_H
#define SEMANTICWORKER_P_H

#include "semanticworker.h"
#include "semantichelper.h"
#include "semanticparser/semanticparser.h"

namespace GrandSearch {
class SemanticWorkerPrivate
{
public:
    struct QueryFunction
    {
        void *object;
        void (*function)(void *object, PushItemCallBack, void *worker);
        SemanticWorkerPrivate *worker;
    };
public:
    explicit SemanticWorkerPrivate(SemanticWorker *parent);
    ~SemanticWorkerPrivate();
    //void tryNotify();
    static bool pushItem(const MatchedItemMap &items, void *ptr);
    static void run(const QueryFunction &func);
    static void sortItems(MatchedItemMap &items, const QHash<QString, int> &weight);
public:
    QString m_serviceName;
    QString m_context;
    QAtomicInt m_status = ProxyWorker::Ready;
    mutable QMutex m_mtx;
    MatchedItemMap m_items;

    //计时
    QTime m_time;
    int m_lastEmit = 0;
private:
    SemanticWorker *q;
};

}

#endif // SEMANTICWORKER_P_H
