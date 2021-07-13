/*
 * Copyright (C) 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhangyu<zhangyub@uniontech.com>
 *
 * Maintainer: zhangyu<zhangyub@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef TASKCOMMANDERPRIVATE_H
#define TASKCOMMANDERPRIVATE_H

#include "taskcommander.h"
#include "searcher/proxyworker.h"

#include <QFuture>
#include <QFutureWatcher>
#include <QReadWriteLock>

class TaskCommanderPrivate : public QObject
{
    Q_OBJECT
    friend class TaskCommander;
public:
    explicit TaskCommanderPrivate(TaskCommander *parent = nullptr);
    ~TaskCommanderPrivate();
    void merge(GrandSearch::MatchedItemMap &addTo, const GrandSearch::MatchedItemMap &addFrom);
private:
    static void working(ProxyWorker *);
private slots:
    void onUnearthed(ProxyWorker *worker);
    void onFinished();
private:
    TaskCommander *q;
    volatile bool m_working = false;
    bool m_deleted = false;
    QString m_id;
    QString m_content;

    //当前所有的搜索结果和新数据缓冲区
    mutable QReadWriteLock m_lock;
    GrandSearch::MatchedItemMap m_results;
    GrandSearch::MatchedItemMap m_buffer;

    QList<ProxyWorker *> m_allWorkers;
    QList<ProxyWorker *> m_asyncWorkers;
    QList<ProxyWorker *> m_syncWorkers;
    QFutureWatcher<void> m_asyncLine;
    QFutureWatcher<void> m_syncLine;
};

#endif // TASKCOMMANDERPRIVATE_H
