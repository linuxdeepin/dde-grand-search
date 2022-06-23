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
#ifndef TASKCOMMANDER_P_H
#define TASKCOMMANDER_P_H

#include "taskcommander.h"
#include "searcher/proxyworker.h"

#include <QFuture>
#include <QFutureWatcher>
#include <QReadWriteLock>

namespace GrandSearch {

class TaskCommanderPrivate : public QObject
{
    Q_OBJECT
    friend class TaskCommander;
public:
    explicit TaskCommanderPrivate(TaskCommander *parent = nullptr);
    ~TaskCommanderPrivate();
    static void merge(MatchedItemMap &addTo, const MatchedItemMap &addFrom);
private:
    static void working(ProxyWorker *);
private slots:
    void onUnearthed(ProxyWorker *worker);
    void onFinished();
    void onWorkFinished(ProxyWorker *worker);
private:
    TaskCommander *q;
    volatile bool m_working = false;
    bool m_deleted = false;
    QString m_id;
    QString m_content;

    //当前所有的搜索结果和新数据缓冲区
    mutable QReadWriteLock m_lock;
    MatchedItemMap m_results;
    MatchedItemMap m_buffer;

    QList<ProxyWorker *> m_allWorkers;
    QList<ProxyWorker *> m_asyncWorkers;
    QList<ProxyWorker *> m_syncWorkers;
    QFutureWatcher<void> m_asyncLine;
    QFutureWatcher<void> m_syncLine;

    //未接收到停止信号的异步woker
    QList<ProxyWorker *> m_workingWorkers;
    bool m_finished = false;  //保证结束信号只发一次
};

}

#endif // TASKCOMMANDER_P_H
